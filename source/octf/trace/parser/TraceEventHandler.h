/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLER_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLER_H

#include <memory>
#include <type_traits>
#include <octf/trace/parser/TraceFileParser.h>
#include <octf/utils/Exception.h>

namespace octf {

/**
 * @brief Base class with methods for handling trace events.
 * @tparam EventType Type of handled events; must be derived from
 * google::protobuf::Message.
 *
 * @note Correct type of template parameter is enforced on compilation time
 * (by static asserts).
 */
template <typename EventType>
class TraceEventHandler {
    static_assert(std::is_base_of<google::protobuf::Message, EventType>(),
                  "Attempted to instantiate template with wrong event type.");

public:
    /**
     * @param  tracePath Path identifying trace (can consist of multiple CPU
     * traces, in particular can point to a set of files)
     */
    TraceEventHandler(const std::string &tracePath)
            : m_message(std::make_shared<EventType>())
            , m_cancelRequested(false) {
        auto cmp = [this](google::protobuf::Message *a,
                          google::protobuf::Message *b) {
            EventType *_a = static_cast<EventType *>(a);
            EventType *_b = static_cast<EventType *>(b);

            return compareEvents(_a, _b);
        };

        m_parser = std::make_shared<TraceFileParser>(
                tracePath, std::make_shared<EventType>(), cmp);
        m_parser->init();

        // TODO (jstencel) Delegate parser creating to some outside class.
    }

    virtual ~TraceEventHandler() {}

    /**
     * @brief Handles single trace event.
     *
     * @param traceEvent A trace event data.
     */
    virtual void handleEvent(std::shared_ptr<EventType> traceEvent) = 0;

    /**
     * @brief Processes all trace events.
     *
     * Depending on lower implementation class, it can be blocking or
     * non-blocking function. In particular, it can periodically check
     * isCancelRequested() method and return accordingly without processing all
     * events.
     *
     * @throws In case of an error Exception is thrown
     */
    virtual void processEvents() {
        // TODO(jstencel) Start background thread with processing
        auto parser = getParser();
        while (!parser->isFinished() && !isCancelRequested()) {
            auto eventMessage = getEventMessagePrototype();
            parser->parseTraceEvent(eventMessage.get());
            handleEvent(eventMessage);
        }
    }

    /**
     * @brief Requests cancellation of processing events.
     *
     * Depending on implementation class, it can take different effect, e.g.
     * stop processing events immediately or after processing some specific part
     * of trace.
     *
     * @note This base class does not provide complete cancellation handling.
     */
    virtual void cancel() {
        m_cancelRequested = true;
    }

    virtual bool compareEvents(const EventType __attribute__((__unused__)) * a,
                               const EventType __attribute__((__unused__)) *
                                       b) {
        return true;
    }

    typedef EventType eventType;

protected:
    /**
     * @brief Checks if cancel of event processing has been requested.
     */
    virtual bool isCancelRequested() const {
        return m_cancelRequested;
    }
    /**
     * @brief Checks if trace event handler has finished.
     */
    virtual bool isFinished() const {
        return m_parser->isFinished();
    }
    /**
     * @brief Gets the prototype of message of specific event type.
     */
    virtual std::shared_ptr<EventType> getEventMessagePrototype() {
        return m_message;
    }
    /**
     * @brief Gets used parser
     */
    virtual std::shared_ptr<ITraceParser> getParser() {
        return m_parser;
    }

private:
    std::shared_ptr<EventType> m_message;
    std::shared_ptr<ITraceParser> m_parser;
    bool m_cancelRequested;
};
}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLER_H
