/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERWORKSET_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERWORKSET_H

#include <octf/analytics/statistics/WorksetCalculator.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

typedef octf::proto::trace::Event Event;
typedef std::shared_ptr<Event> EventShRef;

/**
 * @brief Handler dedicated to CAS events which provides cache size
 * recommendation
 */
class CasTraceEventHandlerWorkset : public octf::TraceEventHandler<Event> {
public:
    /**
     * @see CasTraceEventHandler
     */
    CasTraceEventHandlerWorkset(const std::string &tracePath);

    virtual ~CasTraceEventHandlerWorkset();

    virtual void handleEvent(EventShRef traceEvent) override;

    uint64_t getWorkset();

    bool compareEvents(const Event *a, const Event *b) override {
        return a->header().sid() < b->header().sid();
    }

private:
    octf::WorksetCalculator m_calc;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERWORKSET_H
