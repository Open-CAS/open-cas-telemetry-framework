/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/v2/ParsedIoTraceEventHandler.h>

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
namespace v2 {

struct ParsedIoTraceEventHandler::IoQueueDepth {
    uint64_t Value;
    uint64_t Adjustment;
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

    FileInfo()
            : parent()
            , name() {}

    FileInfo(const proto::trace::EventIoFilesystemFileName &event)
            : parent(event.fileparentid())
            , name(event.filename()) {}

    FileInfo(const FileInfo &other)
            : parent(other.parent)
            , name(other.name) {}

    FileInfo &operator=(const FileInfo &other) {
        if (this != &other) {
            parent = other.parent;
            name = other.name;
        }

        return *this;
    }

    bool operator==(const FileInfo &other) const {
        return name == other.name && parent == other.parent;
    }
};

class ParsedIoTraceEventHandler::FileSystemViewer : public IFileSystemViewer {
public:
    FileSystemViewer(uint64_t partId)
            : IFileSystemViewer()
            , m_partId(partId)
            , m_fileInfo() {}

    void addFile(const FileId &id, const FileInfo &info) {
        m_fileInfo[id] = info;
    }

    bool hasFile(const FileId &id) {
        auto iter = m_fileInfo.find(id);
        return iter != m_fileInfo.end();
    }

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
    std::map<FileId, FileInfo> m_fileInfo;
};

typedef octf::proto::trace::Event Event;
typedef std::shared_ptr<Event> EventShRef;

/**
 * This is a helper class to build a filesystem tree view.
 */
class ParsedIoTraceEventHandler::TraceEventHandlerFilesystemTree
        : public TraceEventHandler<Event> {
public:
    TraceEventHandlerFilesystemTree(const std::string &tracePath)
            : TraceEventHandler<Event>(tracePath) {}
    virtual ~TraceEventHandlerFilesystemTree() = default;

    void init() {
        processEvents();

        // deinitialize parser to release resources
        getParser()->deinit();
    }

    /**
     * Gets filesystem viewer
     *
     * This interface is used to inspect and view filesystem on the basis
     * of captured IO traces.
     *
     * @param partId Partition id of the requested viewer
     *
     * @return Filesystem viewer for specified partition
     */
    FileSystemViewer *getFileSystemViewer(uint64_t partId) {
        FileSystemViewer *viewer = NULL;

        auto iter = m_partitionFsViewers.find(partId);
        if (iter == m_partitionFsViewers.end()) {
            // FS viewer has not be allocated yet.

            // Create FS Viewer
            auto pair = std::make_pair(partId, FileSystemViewer(partId));

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

private:
    virtual void handleEvent(EventShRef traceEvent) override {
        if (traceEvent->has_filesystemfilename()) {
            const auto &fsNameEvent = traceEvent->filesystemfilename();
            FileId id(fsNameEvent);
            FileInfo info(fsNameEvent);

            FileSystemViewer *viewer = getFileSystemViewer(id.partitionId);
            viewer->addFile(id, info);
        }
    }

    bool compareEvents(const Event *a, const Event *b) override {
        return a->header().sid() < b->header().sid();
    }

private:
    std::map<uint64_t, FileSystemViewer> m_partitionFsViewers;
};

constexpr uint64_t ParsedIoTraceEventHandler_QueueLimit = 10000;

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        octf::ParsedIoTraceEventHandler *parentHandler,
        const std::string &tracePath)
        : IoTraceParser(tracePath)
        , m_trace(TraceLibrary::get().getTrace(tracePath))
        , m_queue()
        , m_refSid(0)
        , m_idMapping()
        , m_devices()
        , m_fsTree(new TraceEventHandlerFilesystemTree(tracePath))
        , m_timestampOffset(0)
        , m_limit(ParsedIoTraceEventHandler_QueueLimit)
        , m_subrangeStart(0)
        , m_subrangeEnd(0)
        , m_devIoQueueDepth()
        , m_parentHandler(parentHandler) {
    m_fsTree->init();
}

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
        if (Event::EventTypeCase::kDeviceDescription !=
            traceEvent->EventType_case()) {
            m_timestampOffset = traceEvent->header().timestamp();
        }
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

        // If subrange is set, skip IO events which are outside of it
        if (m_subrangeEnd != 0) {
            if (io.lba() + io.len() < m_subrangeStart ||
                io.lba() > m_subrangeEnd) {
                return;
            }
        }

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
        dst.mutable_flags()->set_direct(src.direct());
        dst.mutable_flags()->set_metadata(src.metadata());
        dst.mutable_flags()->set_readahead(src.readahead());
        dst.set_writehint(src.writehint());

        auto &qd = m_devIoQueueDepth[deviceId];
        qd.Value++;
        dst.set_qd(qd.Value);

        auto *devInfo = cachedEvent.mutable_device();
        devInfo->set_name(m_devices[deviceId].name());
        devInfo->set_id(deviceId);
        devInfo->set_partition(deviceId);
        devInfo->set_model(m_devices[deviceId].model());

        addMapping(*traceEvent, cachedEvent);
    } break;

    case Event::EventTypeCase::kIoCompletion: {
        auto devId = traceEvent->iocompletion().deviceid();
        auto &hdr = traceEvent->header();
        auto &cmpl = traceEvent->iocompletion();

        // If subrange is set, skip events which are outside of it
        if (m_subrangeEnd != 0) {
            if (cmpl.lba() + cmpl.len() < m_subrangeStart ||
                cmpl.lba() > m_subrangeEnd) {
                return;
            }
        }

        // Get queue depth
        auto &qd = m_devIoQueueDepth[devId];

        // Find in map which IO has been completed.
        auto id = cmpl.refid();
        auto event = getCachedEventById(id);

        // If event is null, the submission event probably dropped during
        // tracing
        if (nullptr != event) {
            delMapping(*event);

            auto io = event->mutable_io();
            uint64_t submissionTime = event->header().timestamp();
            uint64_t completionTime = hdr.timestamp();

            // If submission is after completion - IO probably dropped
            if (completionTime >= submissionTime) {
                auto latency = completionTime - submissionTime;

                // IO found, set latency and result of IO
                io->set_latency(latency);
                io->set_error(cmpl.error());

                // Only update these fields if valid - fixes behavior for
                // discards in kernels < 4.10
                if (cmpl.lba() != 0 && cmpl.len() != 0) {
                    io->set_lba(cmpl.lba());
                    io->set_len(cmpl.len());
                }

                // Update queue depth for device
                if (qd.Value) {
                    qd.Value--;
                }
            }
        }

        flushEvents();
    } break;

    case Event::kFilesystemMeta: {
        auto id = traceEvent->filesystemmeta().refid();
        auto cachedEvent = getCachedEventById(id);

        if (cachedEvent) {
            auto &dst = *cachedEvent->mutable_file();
            const auto &src = traceEvent->filesystemmeta();
            dst.set_id(src.fileid().id());
            dst.set_offset(src.fileoffset());
            dst.set_size(src.filesize());

            // Update filesystem event type to just file access
            dst.set_eventtype(proto::trace::FsEventType::Access);

            // Fill creation event
            dst.mutable_creationdate()->CopyFrom(src.fileid().creationdate());

            // Set partition ID
            auto partId = traceEvent->filesystemmeta().fileid().partitionid();
            auto devInfo = cachedEvent->mutable_device();
            devInfo->set_partition(partId);

            // Add device description for given partition
            auto devIter = m_devices.find(partId);
            if (devIter == m_devices.end()) {
                m_devices[partId].CopyFrom(m_devices[devInfo->id()]);
            }
        }
    } break;

    case Event::EventTypeCase::kFilesystemFileEvent: {
        const auto &fsEvent = traceEvent->filesystemfileevent();
        auto partId = fsEvent.fileid().partitionid();

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
        destDevInfo.set_model(srcDevInfo.model());
    } break;

    case Event::EventTypeCase::kFilesystemFileName: {
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

    delMapping(event);

    // Update SID
    event.mutable_header()->set_sid(++m_refSid);

    // Take into account IO queue depth adjustment
    auto devId = event.device().id();
    auto partId = event.device().partition();
    auto &qd = m_devIoQueueDepth[devId];

    if (event.has_io()) {
        auto ioqd = event.io().qd();
        ioqd -= qd.Adjustment;
        event.mutable_io()->set_qd(ioqd);
    }

    // Update timestamp
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

    if (m_trace->getSummary().tags().size()) {
        auto &tags = *event.mutable_extensions()->mutable_tags();
        tags = m_trace->getSummary().tags();
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

void ParsedIoTraceEventHandler::addMapping(
        const proto::trace::Event &traceEvent,
        proto::trace::ParsedEvent &cachedEvent) {
    if (!traceEvent.has_io()) {
        return;
    }
    auto id = traceEvent.io().id();

    if (id) {
        auto pair = std::make_pair(id, &cachedEvent);
        auto result = m_idMapping.emplace(pair);
        if (!result.second) {
            // Probably we dropped completion and we had not deleted previous
            // mapping, replace previous ID mapping with new one
            if (result.first != m_idMapping.end()) {
                result.first->second = &cachedEvent;
            } else {
                throw Exception(
                        "Error during trace parsing, cannot add ID mapping");
            }
        }

        // Temporary store id in SID place
        cachedEvent.mutable_header()->set_sid(id);
    }
}

void ParsedIoTraceEventHandler::delMapping(
        proto::trace::ParsedEvent &cachedEvent) {
    if (cachedEvent.has_io()) {
        auto id = cachedEvent.header().sid();
        m_idMapping.erase(id);
        cachedEvent.mutable_header()->set_sid(0);
    }
}

proto::trace::ParsedEvent *ParsedIoTraceEventHandler::getCachedEventById(
        uint64_t id) {
    auto iter = m_idMapping.find(id);

    if (iter != m_idMapping.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
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

void ParsedIoTraceEventHandler::setExclusiveSubrange(uint64_t start,
                                                     uint64_t end) {
    m_subrangeStart = start;
    m_subrangeEnd = end;
}

IFileSystemViewer *ParsedIoTraceEventHandler::getFileSystemViewer(
        uint64_t partId) {
    return m_fsTree->getFileSystemViewer(partId);
}

}  // namespace v2
}  // namespace trace
}  // namespace octf
