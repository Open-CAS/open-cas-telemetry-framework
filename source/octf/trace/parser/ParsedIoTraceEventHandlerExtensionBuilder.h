/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H

#include <map>
#include <unordered_set>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/defs.pb.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include "octf/interface/ITraceExtensionBuilder.h"

namespace octf {

/**
 * @brief Handler dedicated to get device list from trace
 */
class ParsedIoTraceEventHandlerExtensionBuilder
        : public octf::ParsedIoTraceEventHandler {
public:
    ParsedIoTraceEventHandlerExtensionBuilder(const std::string &tracePath,
                                              ITraceExtensionBuilder *builder);

    virtual ~ParsedIoTraceEventHandlerExtensionBuilder() = default;

    void handleIO(const proto::trace::ParsedEvent &io) override;

private:
    ITraceExtensionBuilder *builder;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H
