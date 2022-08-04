/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H

#include <google/protobuf/util/json_util.h>
#include <map>
#include <unordered_set>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/defs.pb.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/utils/table/Table.h>
#include "octf/interface/ITraceExtensionBuilder.h"

namespace octf {

/**
 * @brief Handler dedicated to get device list from trace
 */
class ParsedIoTraceEventHandlerExtensionBuilder
        : public octf::ParsedIoTraceEventHandler {
public:
    ParsedIoTraceEventHandlerExtensionBuilder(const std::string &tracePath,
                                              ITraceExtensionBuilder *builder,
                                              proto::OutputFormat format);

    virtual ~ParsedIoTraceEventHandlerExtensionBuilder() = default;

    void handleIO(const proto::trace::ParsedEvent &io) override;
    void processEvents() override;

private:
    ITraceExtensionBuilder *builder;
    TraceShRef m_trace;
    table::Table m_table;
    proto::OutputFormat m_format;
    google::protobuf::util::JsonOptions m_jsonOptions;
    std::string m_jsonTrace;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLEREXTENSIONBUILDER_H