/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERJSONPRINTER_H
#define SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERJSONPRINTER_H

#include <octf/proto/trace.pb.h>
#include "TraceEventHandlerJsonPrinter.h"

namespace octf {

class IoTraceEventHandlerJsonPrinter
        : public TraceEventHandlerJsonPrinter<proto::trace::Event> {
public:
    IoTraceEventHandlerJsonPrinter(const std::string &tracePath)
            : TraceEventHandlerJsonPrinter<proto::trace::Event>(tracePath){};
    virtual ~IoTraceEventHandlerJsonPrinter() = default;

    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERJSONPRINTER_H
