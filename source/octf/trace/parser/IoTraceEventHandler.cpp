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
    auto timestamp = traceEvent->header().timestamp();
    if (!m_timestampOffset) {
        // This event handler presents traces from time '0', we remember
        // first event timestamp and subtract by this value for each event
        m_timestampOffset = timestamp;
    }
    traceEvent->mutable_header()->set_timestamp(timestamp - m_timestampOffset);

    switch (traceEvent->EventType_case()) {
    case Event::EventTypeCase::kDeviceDescription: {
        // Remember device
        const auto &device = traceEvent->devicedescription();
        m_devices[device.id()] = device;
    } break;

    case Event::EventTypeCase::kIo: {
        auto sid = traceEvent->header().sid();
        auto deviceId = traceEvent->io().deviceid();
        // Allocate new parsed IO event in the cache
        auto &cachedEvent = m_cache[sid];
        cachedEvent.mutable_header()->CopyFrom(traceEvent->header());

        {
            auto &dst = *cachedEvent.mutable_io();
            const auto &src = traceEvent->io();

            dst.set_lba(src.lba());
            dst.set_len(src.len());
            dst.set_ioclass(src.ioclass());
            dst.set_operation(src.operation());
            dst.set_flush(src.flush());
            dst.set_fua(src.fua());

            dst.set_qd(++m_ioQueueDepth);
        }

        cachedEvent.mutable_device()->set_name(m_devices[deviceId].name());
        cachedEvent.mutable_device()->set_id(deviceId);
    } break;

    case Event::EventTypeCase::kIoCompletion: {
        if (m_ioQueueDepth) {
            m_ioQueueDepth--;
        }

        auto lba = traceEvent->iocompletion().lba();
        auto len = traceEvent->iocompletion().len();
        auto timestamp = traceEvent->header().timestamp();
        auto sid = traceEvent->header().sid();
        auto error = traceEvent->iocompletion().error();
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

            auto io = iter->second.mutable_io();

            if (io->lba() == lba && io->len() == len) {
                uint64_t latency =
                        timestamp - iter->second.header().timestamp();

                // IO found, set latency and result of IO
                io->set_latency(latency);
                io->set_error(error);

                break;
            }
        }
        flushEvents();
    } break;

    case Event::kFilesystemMeta: {
        auto sid = traceEvent->filesystemmeta().refsid();

        auto iter = m_cache.find(sid);
        if (iter != m_cache.end()) {
            auto &dst = *iter->second.mutable_file();
            const auto &src = traceEvent->filesystemmeta();
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

void IoTraceEventHandler::flushEvents() {
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
