/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTQUEUE_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTQUEUE_H

#include <memory>
#include <string>
#include <thread>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * This class is a queue of parsed IO trace events.
 *
 * It parses a trace and allows to access parsed IOs using queue interface.
 *
 * @throws Exception when accessing empty queue (calling front or pop)
 */
class ParsedIoTraceEventQueue : NonCopyable {
public:
    /**
     * @param tracePath Trace path for which queue is created
     */
    ParsedIoTraceEventQueue(const std::string &tracePath);
    virtual ~ParsedIoTraceEventQueue();

    /**
     * @brief Gets parsed event on queue front
     * @return Parsed event on queue front
     */
    const proto::trace::ParsedEvent &front();

    /**
     * @brief Checks if queue is empty (reached end of the event trace)
     *
     * @retval true - queue empty
     * @retval false - queue no empty
     */
    bool empty();

    /**
     * @brief Pops front of the queue
     */
    void pop();

private:
    bool isException();
    void throwException();

private:
    class Parser;
    std::unique_ptr<Parser> m_parser;
    std::thread m_thread;
    Exception m_exception;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTQUEUE_H
