/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERJSONPRINTER_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERJSONPRINTER_H

#include <google/protobuf/util/json_util.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

/**
 * @brief Base class for printing traces to the stream
 *
 * @tparam EventType Type of handled events to be printed. It must be derived
 * from google::protobuf::Message.
 *
 * @note Correct type of template parameter is enforced on compilation time
 * (by static asserts).
 */
template <typename EventType>
class TraceEventHandlerJsonPrinter : public octf::TraceEventHandler<EventType> {
public:
    TraceEventHandlerJsonPrinter(const std::string &tracePath)
            : TraceEventHandler<EventType>(tracePath)
            , m_jsonOptions()
            , m_jsonTrace() {
        m_jsonOptions.always_print_primitive_fields = true;
    }

    virtual ~TraceEventHandlerJsonPrinter() = default;

    void handleEvent(std::shared_ptr<EventType> traceEvent) override {
        m_jsonTrace.clear();
        google::protobuf::util::MessageToJsonString(
                *traceEvent.get(), &m_jsonTrace, m_jsonOptions);

        // TODO (mariuszbarczak) Output stream into which print traces should be
        // defined by constructor
        std::cout << m_jsonTrace << std::endl;
    }

private:
    google::protobuf::util::JsonOptions m_jsonOptions;
    std::string m_jsonTrace;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERJSONPRINTER_H
