/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEFILEPARSER_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEFILEPARSER_H

#include <fstream>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <octf/trace/parser/ITraceParser.h>

namespace octf {

class TraceFileReader;

/**
 * @brief Implementation of trace parser reading trace from files.
 *
 * This class assigns a TraceFileReader object for each trace file
 * to handle reading logic. Then it chooses from which Reader to read
 * next based on the supplied compare function.
 */
class TraceFileParser : public ITraceParser {
public:
    typedef std::function<bool(google::protobuf::Message *,
                               google::protobuf::Message *)>
            CompareFn;
    typedef std::shared_ptr<google::protobuf::Message> MessageShRef;

    virtual ~TraceFileParser();
    /**
     * @param tracePath Indicates directory with trace files,
     * relative to framework trace directory.
     * @param eventPrototype Event message prototype
     * @param compareFn Pointer to function used to compare events.
     * Comparing events determines which events are returned first by parser.
     * 'Smaller' events are returned first.
     */
    TraceFileParser(const std::string &tracePath,
                    MessageShRef eventPrototype,
                    CompareFn compareFn);

    void init() override;

    void deinit() override;

    void parseTraceEvent(google::protobuf::Message *traceEvent) override;

    bool isFinished() const override;

private:
    /**
     * @brief Set element containing pointer to event and its associated queue
     */
    struct EventInfo {
        EventInfo(uint32_t queue, MessageShRef event)
                : queue(queue)
                , event(event) {}
        uint32_t queue;
        MessageShRef event;
    };

    /**
     * @brief Adapter needed to allow using m_compare to sort m_events elements
     */
    struct Comparator {
        Comparator(CompareFn compareFn)
                : m_compare(compareFn){};

        bool operator()(const EventInfo &left, const EventInfo &right) const {
            return m_compare(left.event.get(), right.event.get());
        }

        CompareFn m_compare;
    };

    /**
     * @brief Path to associated trace file
     */
    std::string m_tracePath;

    /**
     * @brief Function used to compare trace events from multiple queues to
     * determine returned events order.
     */
    CompareFn m_compare;

    /**
     * @brief Trace file readers vector
     */
    std::vector<std::unique_ptr<TraceFileReader>> m_readers;

    /**
     * @brief Set containing next event from each not finished reader
     *
     * We utilize set's internal ordering to sort it's elements using a custom
     * comparator. This way set's first element is always
     * the 'smallest'. However upon changing set's element's value we need to
     * reinsert it so that the set remains ordered.
     */
    std::set<EventInfo, Comparator> m_events;

    /**
     * @brief Message prototype for event
     */
    MessageShRef m_eventPrototype;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEFILEPARSER_H
