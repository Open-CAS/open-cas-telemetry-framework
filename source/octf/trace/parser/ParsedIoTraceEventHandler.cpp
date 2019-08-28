/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/parser/ParsedIoTraceEventHandler.h>

#include <list>
#include <map>
#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

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
 * Tiny structure of file name containing device id, file id, parent file id,
 * and name
 *
 * To reduced memory overhead, we introduced own version of FileName,
 * instead of using protocol buffer one
 */
struct ParsedIoTraceEventHandler::FileName {
    uint64_t DeviceId;
    uint64_t Id;
    uint64_t ParentId;
    std::string Name;

    FileName()
            : DeviceId(0)
            , Id(0)
            , ParentId(0)
            , Name() {}

    FileName(const proto::trace::EventIoFilesystemFileName &event)
            : DeviceId(event.deviceid())
            , Id(event.fileid())
            , ParentId(event.fileparentid())
            , Name(event.filename()) {}

    FileName(const FileName &other)
            : DeviceId(other.DeviceId)
            , Id(other.Id)
            , ParentId(other.ParentId)
            , Name(other.Name) {}

    FileName &operator=(const FileName &other) {
        if (this != &other) {
            DeviceId = other.DeviceId;
            Id = other.Id;
            ParentId = other.ParentId;
            Name = other.Name;
        }

        return *this;
    }

    bool operator==(const FileName &other) const {
        return Id == other.Id && DeviceId == other.DeviceId;
    }

    bool operator!=(const FileName &other) const {
        return !(*this == other);
    }

    bool operator<(const FileName &other) const {
        if (DeviceId != other.DeviceId) {
            return DeviceId < other.DeviceId;
        } else {
            return Id < other.Id;
        }
    }
};

constexpr uint64_t ParsedIoTraceEventHandler_QueueLimit = 10000;

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        const std::string &tracePath)
        : TraceEventHandler<proto::trace::Event>(tracePath)
        , m_queue()
        , m_eventMapping(new ParsedIoTraceEventHandler::Map())
        , m_devices()
        , m_fileNames()
        , m_timestampOffset(0)
        , m_sidOffset(0)
        , m_limit(ParsedIoTraceEventHandler_QueueLimit)
        , m_devIoQueueDepth() {}

ParsedIoTraceEventHandler::~ParsedIoTraceEventHandler() {}

void ParsedIoTraceEventHandler::handleEvent(
        std::shared_ptr<proto::trace::Event> traceEvent) {
    using namespace proto::trace;

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
        handleDeviceDescription(device);
    } break;

    case Event::EventTypeCase::kIo: {
        const auto &io = traceEvent->io();
        auto deviceId = io.deviceid();

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
        dst.set_ioclass(src.ioclass());
        dst.set_operation(src.operation());
        dst.set_flush(src.flush());
        dst.set_fua(src.fua());
        dst.set_writehint(src.writehint());

        auto &qd = m_devIoQueueDepth[deviceId];
        qd.Value++;
        dst.set_qd(qd.Value);

        cachedEvent.mutable_device()->set_name(m_devices[deviceId].name());
        cachedEvent.mutable_device()->set_id(deviceId);

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
                throw Exception(
                        "Trace file error, completion time before "
                        "submission");
            }

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
            dst.set_id(src.fileid());
            dst.set_offset(src.fileoffset());
            dst.set_size(src.filesize());
        }
    } break;

    case Event::kFilesystemFileNameFieldNumber: {
        // TODO (mariuszbarczak) set limit on m_fileNames

        // Create SID mapping
        auto sidResult =
                m_fileNames.emplace(FileName(traceEvent->filesystemfilename()));

        if (!sidResult.second) {
            // Equivalent element already exist

            if (sidResult.first == m_fileNames.end()) {
                // Iterator shall point at the existing element
                throw Exception(
                        "Error during trace parsing, cannot cache file name");
            }

            // Exchange old one with new one
            m_fileNames.erase(sidResult.first);
            m_fileNames.insert(FileName(traceEvent->filesystemfilename()));
        }
    } break;

    default: { } break; }
}

std::shared_ptr<proto::trace::Event>
ParsedIoTraceEventHandler::getEventMessagePrototype() {
    return std::make_shared<proto::trace::Event>();
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
    auto &qd = m_devIoQueueDepth[devId];
    auto ioqd = event.io().qd();
    if (qd.Adjustment < ioqd) {
        ioqd -= qd.Adjustment;
    } else {
        ioqd = 1;
    }
    event.mutable_io()->set_qd(ioqd);

    // Update latency
    auto timestamp = event.header().timestamp();
    if (timestamp > m_timestampOffset) {
        event.mutable_header()->set_timestamp(timestamp -= m_timestampOffset);
    } else {
        event.mutable_header()->set_timestamp(0);
    }

    if (event.has_file()) {
        getFilePath(devId, event.file().id(),
                    *event.mutable_file()->mutable_path());
    }

    // Call handler
    handleIO(event);

    if (0 == event.io().latency()) {
        // An IO completion lost, so the queue depth of next IOs are disrupted,
        // Set queue depth adjustment
        qd.Adjustment++;
    }

    m_queue.pop();
}

void ParsedIoTraceEventHandler::getFilePath(uint64_t devId,
                                            uint64_t id,
                                            std::string &path) {
    // TODO (mariuszbarczak) Provide path caching with limit

    FileName key;
    key.Id = id;
    key.DeviceId = devId;

    auto iter = m_fileNames.find(key);
    if (iter != m_fileNames.end()) {
        const auto &name = *iter;

        if (name.Id != name.ParentId) {
            getFilePath(devId, name.ParentId, path);
            path += "/";
            path += name.Name;
        }
    } else {
        if (path != "") {
            path += "../";
        }
    }
}

}  // namespace octf
