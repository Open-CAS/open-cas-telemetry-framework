/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/ParsedIoTraceEventHandlerPrinter.h>

namespace octf {

ParsedIoTraceEventHandlerPrinter::ParsedIoTraceEventHandlerPrinter(
        const std::string &tracePath,
        proto::OutputFormat format)
        : ParsedIoTraceEventHandler(tracePath)
        , m_trace(TraceLibrary::get().getTrace(tracePath))
        , m_table()
        , m_format(format)
        , m_jsonOptions()
        , m_jsonTrace() {
    m_jsonOptions.always_print_primitive_fields = true;
    m_jsonOptions.add_whitespace = false;
}

void ParsedIoTraceEventHandlerPrinter::handleIO(
        const proto::trace::ParsedEvent &io) {
    switch (m_format) {
    case proto::OutputFormat::CSV: {
        m_table[0].clear();
        m_table[0] << io;
        std::cout << m_table << std::endl;
    } break;
    case proto::OutputFormat::JSON: {
        m_jsonTrace.clear();
        auto status = google::protobuf::util::MessageToJsonString(io, &m_jsonTrace,
                                                             m_jsonOptions);
        if (!status.ok()) {
            throw Exception("Failed to convert event to JSON");
        }
        std::cout << m_jsonTrace << std::endl;
    } break;
    default: {
        throw Exception("Invalid output format");
    } break;
    }
}

void ParsedIoTraceEventHandlerPrinter::processEvents() {
    if (m_format == proto::OutputFormat::CSV) {
        // Form CSV header and print it
        proto::trace::ParsedEvent event;

        /* Populate tags */
        auto &tags = *event.mutable_extensions()->mutable_tags();
        tags = m_trace->getSummary().tags();

        table::setHeader(m_table[0], &event);
        std::cout << m_table << std::endl;
    }
    ParsedIoTraceEventHandler::processEvents();
}

}  // namespace octf
