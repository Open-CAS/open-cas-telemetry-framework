/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERCSVPRINER_H
#define SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERCSVPRINER_H

#include <octf/proto/trace.pb.h>
#include "TraceEventHandlerCsvPrinter.h"

namespace octf {

class IoTraceEventHandlerCsvPrinter
        : public TraceEventHandlerCsvPrinter<proto::trace::Event> {
public:
    IoTraceEventHandlerCsvPrinter(const std::string &tracePath)
            : TraceEventHandlerCsvPrinter<proto::trace::Event>(tracePath){};
    virtual ~IoTraceEventHandlerCsvPrinter() = default;

    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_IOTRACEEVENTHANDLERCSVPRINER_H
