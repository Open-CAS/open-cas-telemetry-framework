/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/parser/IoTraceEventHandler.h>

namespace octf {

IoTraceEventHandler::IoTraceEventHandler(const std::string &tracePath)
        : TraceEventHandler<proto::trace::Event>(tracePath)
        , m_cache()
        , m_devices()
        , m_timestampOffset(0)
        , m_ioQueueDepth(0) {}

void IoTraceEventHandler::handleEvent(
        std::shared_ptr<proto::trace::Event> traceEvent) {
    using namespace proto::trace;
    auto timestamp = traceEvent->mutable_header()->timestamp();
    if (!m_timestampOffset) {
        // This event handler presents traces from time '0', we remember
        // first event timestamp and subtract by this value for each event
        m_timestampOffset = timestamp;
    }
    traceEvent->mutable_header()->set_timestamp(timestamp - m_timestampOffset);

    switch (traceEvent->EventType_case()) {
    case Event::EventTypeCase::kDeviceDescription: {
        // Remember device
        auto device = traceEvent->mutable_devicedescription();
        m_devices[device->id()] = *device;
    } break;

    case Event::EventTypeCase::kIo: {
        auto sid = traceEvent->mutable_header()->sid();
        auto device = traceEvent->io().deviceid();
        // Allocate new parsed IO event in the cache
        auto &cachedEvent = m_cache[sid];
        cachedEvent.mutable_header()->CopyFrom(traceEvent->header());

        {
            auto &dst = *cachedEvent.mutable_io();
            const auto &src = traceEvent->io();

            dst.set_lba(src.lba());
            dst.set_len(src.len());
            dst.set_ioclass(src.ioclass());
            dst.set_deviceid(src.deviceid());
            dst.set_operation(src.operation());
            dst.set_flush(src.flush());
            dst.set_fua(src.fua());

            dst.set_qd(++m_ioQueueDepth);
        }

        cachedEvent.mutable_device()->set_name(m_devices[device].name());
    } break;

    case Event::EventTypeCase::kIoCompletion: {
        if (m_ioQueueDepth) {
            m_ioQueueDepth--;
        }

        auto lba = traceEvent->mutable_iocompletion()->lba();
        auto len = traceEvent->mutable_iocompletion()->len();
        auto timestamp = traceEvent->mutable_header()->timestamp();
        auto sid = traceEvent->mutable_header()->sid();
        auto error = traceEvent->mutable_iocompletion()->error();
        // Find in cache which IO has been completed. We match by LBA and
        // length
        auto iter = m_cache.begin();
        auto end = m_cache.end();
        for (; iter != end; ++iter) {
            if (iter->first > sid) {
                // Iterating over higher SID, what means over never event,
                // so
                break;
            }

            if (iter->second.mutable_io()->lba() == lba &&
                iter->second.mutable_io()->len() == len) {
                uint64_t latency =
                        timestamp - iter->second.mutable_header()->timestamp();

                // IO found, set latency and result of IO
                iter->second.mutable_io()->set_latency(latency);
                iter->second.mutable_io()->set_error(error);

                break;
            }
        }
        flushEvetns();
    } break;

    case Event::kFilesystemMeta: {
        auto sid = traceEvent->mutable_filesystemmeta()->refsid();

        auto iter = m_cache.find(sid);
        if (iter != m_cache.end()) {
            auto &dst = *iter->second.mutable_file();
            const auto &src = *traceEvent->mutable_filesystemmeta();
            dst.set_id(src.fileid());
            dst.set_offset(src.fileoffset());
            dst.set_size(src.filesize());
        }
    } break;

    default: { } break; }
}

std::shared_ptr<proto::trace::Event>
IoTraceEventHandler::getEventMessagePrototype() {
    return std::make_shared<proto::trace::Event>();
}

void IoTraceEventHandler::flushEvetns() {
    auto iter = m_cache.begin();
    auto end = m_cache.end();
    for (; iter != end;) {
        if (iter->second.mutable_io()->latency()) {
            handleIO(iter->second);
            iter = m_cache.erase(iter);
        } else {
            break;
        }
    }

    // If IO traces cache exceed some number, or all parser finished its job,
    // flush IOs
    constexpr uint64_t cacheLimit = 1000 * 1000;
    while (m_cache.size() > cacheLimit || getParser()->isFinished()) {
        handleIO(m_cache.begin()->second);
        m_cache.erase(m_cache.begin());

        if (0 == m_cache.size()) {
            break;
        }
    }
}

}  // namespace octf
