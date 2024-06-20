/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERPRINTER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERPRINTER_H

#include <google/protobuf/util/json_util.h>
#include <octf/proto/defs.pb.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/utils/table/Table.h>

namespace octf {

/**
 * @brief Printer of Parsed IO
 */
class ParsedIoTraceEventHandlerPrinter : public ParsedIoTraceEventHandler {
public:
    ParsedIoTraceEventHandlerPrinter(const std::string &tracePath,
                                     proto::OutputFormat format);

    virtual ~ParsedIoTraceEventHandlerPrinter() = default;

    void handleIO(const proto::trace::ParsedEvent &io) override;

    void processEvents() override;

private:
    TraceShRef m_trace;
    table::Table m_table;
    proto::OutputFormat m_format;
    google::protobuf::util::JsonPrintOptions m_jsonOptions;
    std::string m_jsonTrace;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERPRINTER_H
