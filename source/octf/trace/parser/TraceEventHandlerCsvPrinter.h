/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERCSVPRINTER_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERCSVPRINTER_H

#include <google/protobuf/util/json_util.h>
#include <iostream>
#include <octf/trace/parser/TraceEventHandler.h>
#include <octf/utils/table/Table.h>

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
class TraceEventHandlerCsvPrinter : public TraceEventHandler<EventType> {
public:
    TraceEventHandlerCsvPrinter(const std::string &tracePath)
            : TraceEventHandler<EventType>(tracePath)
            , m_table() {}

    virtual ~TraceEventHandlerCsvPrinter() = default;

    void handleEvent(std::shared_ptr<EventType> traceEvent) override {
        m_table[0].clear();
        m_table[0] << *traceEvent;
        std::cout << m_table << std::endl;
    }

    void processEvents() override {
        // Form CSV header and print it
        auto msg = TraceEventHandler<EventType>::getEventMessagePrototype();
        table::setHeader(m_table[0], msg.get());
        std::cout << m_table << std::endl;

        TraceEventHandler<EventType>::processEvents();
    }

private:
    table::Table m_table;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERCSVPRINTER_H
