/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H

#include <map>
#include <memory>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

/**
 * This is IO trace event handler of parsed IO
 *
 * The parsed IO is created from multiple events (especially IO request and IO
 * completion). It contains basic IO information (LBA, length, etc). It is
 * supplemented by related information like filesystem one. In addition it
 * provides post parse information (latency, queue depth, etc...).
 *
 * @note The order of handled IO respect the IOs queuing order
 */
class ParsedIoTraceEventHandler
        : public TraceEventHandler<proto::trace::Event> {
public:
    ParsedIoTraceEventHandler(const std::string &tracePath);
    virtual ~ParsedIoTraceEventHandler() = default;

    /**
     * Handles parsed IO
     *
     * @param IO Parsed IO to be handle
     */
    virtual void handleIO(proto::trace::ParsedEvent &io) = 0;

    void processEvents() override {
        TraceEventHandler<proto::trace::Event>::processEvents();
        flushEvents();
    }

private:
    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }

    void handleEvent(std::shared_ptr<proto::trace::Event> traceEvent) override;

    virtual std::shared_ptr<proto::trace::Event> getEventMessagePrototype()
            override;

    void flushEvents();

private:
    std::map<uint64_t, proto::trace::ParsedEvent> m_cache;
    std::map<uint64_t, proto::trace::EventDeviceDescription> m_devices;
    uint64_t m_timestampOffset;
    uint64_t m_ioQueueDepth;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
