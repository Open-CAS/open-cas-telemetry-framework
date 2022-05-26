/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/v0/ParsedIoTraceEventHandler.h>

#include <limits.h>
#include <chrono>
#include <list>
#include <map>
#include <octf/fs/FileId.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace trace {
namespace v0 {

/**
 * Metadata IO, for example filesystem one
 */
#define OCTF_IO_CLASS_METADATA 1
/**
 * Direct IO, for example IO which bypasses memory cache
 */
#define OCTF_IO_CLASS_DIRECT 22

struct ParsedIoTraceEventHandler::IoQueueDepth {
    uint64_t Value;
    uint64_t Adjustment;
};

struct ParsedIoTraceEventHandler::Key {
    Key(const proto::trace::Event &event)
            : DeviceId(event.io().deviceid())
            , Lba(event.io().lba())
            , Len(event.io().len()) {}

    Key(const proto::trace::EventIoCompletion &cmpl)
            : DeviceId(cmpl.deviceid())
            , Lba(cmpl.lba())
            , Len(cmpl.len()) {}

    Key(const proto::trace::ParsedEvent &event)
            : DeviceId(event.device().id())
            , Lba(event.io().lba())
            , Len(event.io().len()) {}

    Key()
            : DeviceId(0)
            , Lba(0)
            , Len(0) {}

    virtual ~Key() {}

    Key(const Key &other)
            : DeviceId(other.DeviceId)
            , Lba(other.Lba)
            , Len(other.Len) {}

    Key &operator=(const Key &other) {
        if (this != &other) {
            DeviceId = other.DeviceId;
            Lba = other.Lba;
            Len = other.Len;
        }

        return *this;
    }

    bool operator==(const Key &other) const {
        return Lba == other.Lba && Len == other.Len &&
               DeviceId == other.DeviceId;
    }

    bool operator!=(const Key &other) const {
        return !(*this == other);
    }

    bool operator<(const Key &other) const {
        if (DeviceId != other.DeviceId) {
            return DeviceId < other.DeviceId;
        } else if (Lba != other.Lba) {
            return Lba < other.Lba;
        } else {
            return Len < other.Len;
        }
    }

    uint64_t DeviceId;
    uint64_t Lba;
    uint32_t Len;
};

class ParsedIoTraceEventHandler::Map : public NonCopyable {
public:
    Map() = default;
    virtual ~Map() = default;

    typedef proto::trace::ParsedEvent *MapEvent;

    void add(const Key &key, MapEvent event) {
        auto iter = m_map.find(key);
        if (iter == m_map.end()) {
            auto result =
                    m_map.emplace(std::make_pair(key, std::list<MapEvent>()));
            if (!result.second || result.first == m_map.end()) {
                throw Exception(
                        "Error during trace parsing, cannot cache trace event");
            }
            iter = result.first;
        }

        iter->second.push_back(event);
    }

    /**
     * @brief Gets MapEvent for specified key and for which latency has not been
     * set
     *
     * @param key Key of requested MapEvent
     * @return MapEvent
     */
    MapEvent get(const Key &key) {
        auto iter = m_map.find(key);

        if (iter != m_map.end()) {
            for (auto event : iter->second) {
                if (0 == event->io().latency()) {
                    // latency equals zero means not set yet
                    return event;
                }
            }
        }

        return nullptr;
    }

    void erase(const Key &key, uint64_t sid) {
        auto mapIter = m_map.find(key);

        if (mapIter != m_map.end()) {
            auto &lst = mapIter->second;

            auto lstIter = lst.begin();
            for (; lstIter != lst.end(); ++lstIter) {
                if ((*lstIter)->header().sid() == sid) {
                    lst.erase(lstIter);
                    break;
                }
            }

            if (lst.empty()) {
                m_map.erase(mapIter);
            }
        }
    }

private:
    std::map<Key, std::list<MapEvent>> m_map;
};

/**
 * Tiny structure of file info containing parent file id, last size of file,
 * name, etc.
 *
 * To reduced memory overhead, we introduced own version of file info,
 * instead of using protocol buffer one
 */
struct ParsedIoTraceEventHandler::FileInfo {
    FileId parent;
    std::string name;
    uint64_t size;

    FileInfo()
            : parent()
            , name()
            , size(0) {}

    FileInfo(const proto::trace::EventIoFilesystemFileName &event)
            : parent(event)
            , name(event.filename())
            , size(0) {}

    FileInfo(const FileInfo &other)
            : parent(other.parent)
            , name(other.name)
            , size(other.size) {}

    FileInfo &operator=(const FileInfo &other) {
        if (this != &other) {
            parent = other.parent;
            name = other.name;
            size = other.size;
        }

        return *this;
    }
};

constexpr uint64_t ParsedIoTraceEventHandler_QueueLimit = 10000;

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        octf::ParsedIoTraceEventHandler *parentHandler,
        const std::string &tracePath)
        : IoTraceParser(tracePath)
        , m_queue()
        , m_eventMapping(new ParsedIoTraceEventHandler::Map())
        , m_devices()
        , m_fileInfo()
        , m_timestampOffset(0)
        , m_sidOffset(0)
        , m_limit(ParsedIoTraceEventHandler_QueueLimit)
        , m_subrangeStart(0)
        , m_subrangeEnd(0)
        , m_devIoQueueDepth()
        , m_parentHandler(parentHandler) {}

ParsedIoTraceEventHandler::~ParsedIoTraceEventHandler() {}

void ParsedIoTraceEventHandler::processEvents() {
    TraceEventHandler<proto::trace::Event>::processEvents();
    flushEvents();
}

void ParsedIoTraceEventHandler::handleEvent(
        std::shared_ptr<proto::trace::Event> traceEvent) {
    using namespace proto::trace;
    using octf::FileId;

    if (!m_timestampOffset) {
        // This event handler presents traces from time '0', and SID '0',
        // we remember the first event time stamp and SID and the subtract
        // by those values for each event
        m_timestampOffset = traceEvent->header().timestamp();
        m_sidOffset = traceEvent->header().sid();
    }

    switch (traceEvent->EventType_case()) {
    case Event::EventTypeCase::kDeviceDescription: {
        // Remember device
        const auto &device = traceEvent->devicedescription();
        m_devices[device.id()] = device;
        m_limit = ParsedIoTraceEventHandler_QueueLimit * m_devices.size();
        m_parentHandler->handleDeviceDescription(device);
    } break;

    case Event::EventTypeCase::kIo: {
        const auto &io = traceEvent->io();
        auto deviceId = io.deviceid();

        // If subrange is set, skip io events which are outside of it
        if (m_subrangeEnd != 0) {
            if (io.lba() + io.len() < m_subrangeStart ||
                io.lba() > m_subrangeEnd) {
                return;
            }
        }

        // Create key
        Key key(*traceEvent);

        // Allocate new parsed IO event in the queue
        m_queue.emplace(ParsedEvent());
        auto &cachedEvent = m_queue.back();

        // Setup parsed IO
        cachedEvent.mutable_header()->CopyFrom(traceEvent->header());

        auto &dst = *cachedEvent.mutable_io();
        const auto &src = traceEvent->io();

        dst.set_lba(src.lba());
        dst.set_len(src.len());
        dst.set_operation(src.operation());
        dst.mutable_flags()->set_flush(src.flush());
        dst.mutable_flags()->set_fua(src.fua());
        dst.set_writehint(src.writehint());

        if (src.ioclass() == OCTF_IO_CLASS_METADATA) {
            dst.mutable_flags()->set_metadata(true);
        } else if (src.ioclass() == OCTF_IO_CLASS_DIRECT) {
            dst.mutable_flags()->set_direct(true);
        }

        auto &qd = m_devIoQueueDepth[deviceId];
        qd.Value++;
        dst.set_qd(qd.Value);

        auto *devInfo = cachedEvent.mutable_device();
        devInfo->set_name(m_devices[deviceId].name());
        devInfo->set_id(deviceId);
        devInfo->set_partition(deviceId);

        // Create LBA mapping
        m_eventMapping->add(key, &cachedEvent);

        // Create SID mapping
        auto sidResult = m_sidMapping.emplace(
                std::make_pair(traceEvent->header().sid(), &cachedEvent));
        if (!sidResult.second || sidResult.first == m_sidMapping.end()) {
            throw Exception(
                    "Error during trace parsing, cannot cache trace event");
        }
    } break;

    case Event::EventTypeCase::kIoCompletion: {
        auto devId = traceEvent->iocompletion().deviceid();
        auto complEvent = traceEvent->iocompletion();

        // If subrange is set, skip events which are outside of it
        if (m_subrangeEnd != 0) {
            if (complEvent.lba() + complEvent.len() < m_subrangeStart ||
                complEvent.lba() > m_subrangeEnd) {
                return;
            }
        }

        // Create key
        Key key(traceEvent->iocompletion());

        // Get queue depth
        auto &qd = m_devIoQueueDepth[devId];

        // Find in map which IO has been completed. We match by IO LBA, IO
        // length, and IO device

        auto event = m_eventMapping->get(key);
        if (nullptr != event) {
            auto io = event->mutable_io();
            uint64_t submissionTime = event->header().timestamp();
            uint64_t completionTime = traceEvent->header().timestamp();

            if (completionTime < submissionTime) {
                // Submission after completion - IO event to same lba was
                // probably dropped
                if (qd.Adjustment) {
                    qd.Adjustment--;
                }
            } else {
                uint64_t latency = completionTime - submissionTime;

                // IO found, set latency and result of IO
                io->set_latency(latency);
                io->set_error(traceEvent->iocompletion().error());

                // Update queue depth for device
                if (qd.Value) {
                    qd.Value--;

                    if (qd.Value < qd.Adjustment) {
                        // taking into account adjustment, we reached zero queue
                        // depth, so reset both counters
                        qd.Value = 0;
                        qd.Adjustment = 0;
                    }
                }
            }
        } else {
            // IO not found, probably event dropped during tracing
            if (qd.Adjustment) {
                qd.Adjustment--;
            }
        }

        flushEvents();
    } break;

    case Event::kFilesystemMeta: {
        auto sid = traceEvent->filesystemmeta().refsid();

        auto iter = m_sidMapping.find(sid);
        if (iter != m_sidMapping.end()) {
            auto &dst = *iter->second->mutable_file();
            const auto &src = traceEvent->filesystemmeta();
            dst.set_id(src.fileid().id());
            dst.set_offset(src.fileoffset());
            dst.set_size(src.filesize());

            // Update filesystem event type to just file access
            dst.set_eventtype(proto::trace::FsEventType::Access);
            dst.mutable_creationdate()->CopyFrom(src.fileid().creationdate());

            // Set partition ID
            auto partId = traceEvent->filesystemmeta().fileid().partitionid();
            auto devInfo = iter->second->mutable_device();
            devInfo->set_partition(partId);

            // Add device description for given partition
            auto devIter = m_devices.find(partId);
            if (devIter == m_devices.end()) {
                m_devices[partId].CopyFrom(m_devices[devInfo->id()]);
            }

            // Update size in file info
            auto size = src.filesize();
            m_fileInfo[FileId(src)].size = size;
        }
    } break;

    case Event::EventTypeCase::kFilesystemFileEvent: {
        const auto &fsEvent = traceEvent->filesystemfileevent();
        auto partId = fsEvent.fileid().partitionid();
        FileId file = FileId(fsEvent);

        // Allocate new parsed IO event in the queue
        m_queue.emplace(ParsedEvent());
        auto &cachedEvent = m_queue.back();

        // Setup parsed IO
        cachedEvent.mutable_header()->CopyFrom(traceEvent->header());

        // Setup file event type and parent id
        auto &dstFileInfo = *cachedEvent.mutable_file();
        dstFileInfo.set_eventtype(fsEvent.fseventtype());
        dstFileInfo.set_id(fsEvent.fileid().id());
        dstFileInfo.mutable_creationdate()->CopyFrom(
                fsEvent.fileid().creationdate());

        auto &destDevInfo = *cachedEvent.mutable_device();
        const auto &srcDevInfo = m_devices[partId];
        destDevInfo.set_name(srcDevInfo.name());
        destDevInfo.set_id(srcDevInfo.id());
        destDevInfo.set_partition(partId);

        // Set file size from file info
        auto size = m_fileInfo[file].size;
        dstFileInfo.set_size(size);
        if (fsEvent.fseventtype() == FsEventType::Delete) {
            m_fileInfo.erase(file);
        }
    } break;

    case Event::EventTypeCase::kFilesystemFileName: {
        const auto &fsNameEvent = traceEvent->filesystemfilename();
        FileId fileId(fsNameEvent);
        auto &fileInfo = m_fileInfo[fileId];

        fileInfo.name = fsNameEvent.filename();
        fileInfo.parent = FileId(fsNameEvent.fileparentid());
    } break;

    default:
        break;
    }
}

void ParsedIoTraceEventHandler::flushEvents() {
    while (m_queue.size()) {
        if (m_queue.front().io().latency()) {
            pushOutEvent();
        } else {
            break;
        }
    }

    bool isFinished = getParser()->isFinished();

    if (m_queue.size() < m_limit && !isFinished) {
        return;
    }

    // Cache exceeds some number,
    // or every parser finished its job, then flush IOs
    while (m_queue.size()) {
        pushOutEvent();

        if (!isFinished && m_queue.size() < m_limit) {
            break;
        }
    }
}

void ParsedIoTraceEventHandler::pushOutEvent() {
    auto &event = m_queue.front();

    // Get SID
    auto sid = event.header().sid();

    // Remove SID mapping
    m_sidMapping.erase(sid);

    // Remove LBA mapping
    Key key(event);
    m_eventMapping->erase(key, sid);

    // Update SID
    event.mutable_header()->set_sid(sid - m_sidOffset);

    // Take into account IO queue depth adjustment
    auto devId = event.device().id();
    auto partId = event.device().partition();
    auto &qd = m_devIoQueueDepth[devId];

    if (event.has_io()) {
        auto ioqd = event.io().qd();
        if (qd.Adjustment < ioqd) {
            ioqd -= qd.Adjustment;
        } else {
            ioqd = 1;
        }
        event.mutable_io()->set_qd(ioqd);
    }

    // Update latency
    auto timestamp = event.header().timestamp();
    if (timestamp > m_timestampOffset) {
        event.mutable_header()->set_timestamp(timestamp -= m_timestampOffset);
    } else {
        event.mutable_header()->set_timestamp(0);
    }

    if (event.has_file()) {
        auto viewer = getFileSystemViewer(partId);
        event.mutable_file()->set_path(viewer->getFilePath(FileId(event)));
    }

    // Call handler
    m_parentHandler->handleIO(event);

    if (event.has_io() && 0 == event.io().latency()) {
        // An IO completion lost, so the queue depth of next IOs are disrupted,
        // Set queue depth adjustment
        qd.Adjustment++;
    }

    m_queue.pop();
}

void ParsedIoTraceEventHandler::setExclusiveSubrange(uint64_t start,
                                                     uint64_t end) {
    m_subrangeStart = start;
    m_subrangeEnd = end;
}

class ParsedIoTraceEventHandler::FileSystemViewer : public IFileSystemViewer {
public:
    FileSystemViewer(uint64_t partId,
                     const std::map<FileId, FileInfo> &fileInfo)
            : IFileSystemViewer()
            , m_partId(partId)
            , m_fileInfo(fileInfo) {}

    virtual std::string getFileNamePrefix(const FileId &id) const override {
        std::string basename = "";

        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            auto i = iter->second.name.rfind('.');
            if (i != std::string::npos) {
                basename = iter->second.name.substr(0, i);
            } else {
                basename = iter->second.name;
            }
        }

        while (basename.size()) {
            if (std::isalpha(basename.back())) {
                break;
            } else {
                basename.pop_back();
            }
        }

        return basename;
    }

    virtual std::string getFileName(const FileId &id) const override {
        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            return iter->second.name;
        }

        return "";
    }

    virtual std::string getFileExtension(const FileId &id) const override {
        std::string extension = "";

        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            auto i = iter->second.name.rfind('.');
            if (i != std::string::npos) {
                extension = iter->second.name.substr(i + 1);
            }
        }

        return extension;
    }

    virtual std::string getDirPath(const FileId &id) const override {
        std::string dir = "";
        uint64_t len = 0;

        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            try {
                getPath(iter->second.parent, dir, len);
            } catch (MaxPathExceededException &e) {
                log::cerr << e.getMessage() << std::endl;
            }
        }

        return dir;
    }

    virtual std::string getFilePath(const FileId &id) const override {
        std::string path = "";

        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            path = getDirPath(id);

            if ("" == path) {
                return "";
            }

            if (path != "/") {
                path += "/";
            }

            path += iter->second.name;
        }

        return path;
    }

    virtual FileId getParentId(const FileId &id) const override {
        FileId parentId = FileId();

        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            const auto &info = iter->second;
            return info.parent;
        }

        return parentId;
    }

private:
    bool getPath(const FileId &id, std::string &path, uint64_t &len) const {
        auto iter = m_fileInfo.find(id);
        if (iter != m_fileInfo.end()) {
            const auto &info = iter->second;
            len += info.name.length();
            if (len > PATH_MAX) {
                throw MaxPathExceededException(id.id);
            }
            if (id != info.parent) {
                if (!getPath(info.parent, path, len)) {
                    path = "";
                    return false;
                }

                if (!path.empty() && '/' != path.back()) {
                    path += "/";
                }
            }

            path += info.name;

            return true;
        } else {
            path = "";
            return false;
        }
    }

private:
    const uint64_t m_partId;
    const std::map<FileId, FileInfo> &m_fileInfo;
};

IFileSystemViewer *ParsedIoTraceEventHandler::getFileSystemViewer(
        uint64_t partitionId) {
    IFileSystemViewer *viewer = NULL;

    // Check if device with specified ID exist
    if (m_devices.end() == m_devices.find(partitionId)) {
        throw Exception("Requesting FS viewer for non-existing partition ID");
    }

    auto iter = m_partitionFsViewers.find(partitionId);
    if (iter == m_partitionFsViewers.end()) {
        // FS viewer has not be allocated yet.

        // Create FS Viewer
        auto pair = std::make_pair(partitionId,
                                   FileSystemViewer(partitionId, m_fileInfo));

        // Insert pair into map
        auto result = m_partitionFsViewers.emplace(pair);

        if (!result.second || result.first == m_partitionFsViewers.end()) {
            throw Exception(
                    "Error during trace parsing, cannot create FS viewer");
        }

        viewer = &result.first->second;
    } else {
        viewer = &iter->second;
    }

    return viewer;
}

uint64_t ParsedIoTraceEventHandler::getDevicesSize() const {
    uint64_t size = 0;

    for (const auto &dev : m_devices) {
        // In device map we code partition sizes as well, so peek only entire
        // drives, takes place when key of map equals to id in map value.
        if (dev.first == dev.second.id()) {
            size += dev.second.size();
        }
    }

    return size;
}

}  // namespace v0
}  // namespace trace
}  // namespace octf
