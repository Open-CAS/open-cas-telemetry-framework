/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/trace/parser/TraceFileParser.h>

#include <sstream>
#include <octf/interface/TraceManager.h>
#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/parser/TraceFileReader.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

using namespace std;

TraceFileParser::~TraceFileParser() {
    deinit();
}

TraceFileParser::TraceFileParser(const string &tracePath,
                                 MessageShRef eventPrototype,
                                 CompareFn comp)
        : ITraceParser()
        , m_tracePath(tracePath)
        , m_compare(comp)
        , m_readers()
        , m_events(Comparator(comp))
        , m_eventPrototype(eventPrototype) {}

void TraceFileParser::init() {
    // Read summary in trace location
    proto::TraceSummary summary;
    stringstream traceLocation;
    traceLocation << getFrameworkConfiguration().getTraceDir() << "/"
                  << m_tracePath;
    ProtobufReaderWriter rw(traceLocation.str() + "/" + SUMMARY_FILE_NAME);
    if (!rw.read(summary)) {
        throw Exception(
                "Could not parse specified trace. No summary file found.");
    }

    if (summary.state() != proto::TraceState::COMPLETE) {
        throw Exception("Specified trace is not in 'complete' state.");
    }

    // Create reader for each queue
    if (summary.queuecount() <= 0) {
        throw Exception("Trace summary contains invalid values.");
    }

    string traceFilePath;
    for (uint32_t queue = 0; queue < summary.queuecount(); queue++) {
        traceFilePath = "";
        traceFilePath = traceLocation.str() + "/" + TRACE_FILE_PREFIX +
                        to_string(queue);
        m_readers.push_back(unique_ptr<TraceFileReader>(
                new TraceFileReader(traceFilePath, queue)));

        m_readers.back()->init();
    }

    // Initialize events container
    for (uint32_t i = 0; i < m_readers.size(); i++) {
        if (m_readers[i]->isFinished()) {
            // Trace file is empty
            continue;
        }
        MessageShRef event(m_eventPrototype->New());
        m_readers[i]->readTraceEvent(event);
        m_events.insert(EventInfo(i, event));
    }
}

void TraceFileParser::deinit() {
    // Close readers
    for (auto &reader : m_readers) {
        reader->deinit();
    }
    m_readers.clear();
}

void TraceFileParser::parseTraceEvent(google::protobuf::Message *traceEvent) {
    if (m_events.empty()) {
        throw Exception(
                "Attempted to parse event using parser"
                " which has finished or wasn't initialized");
    }

    // Copy 'smallest' event
    // [TODO] (trybicki) C++17 feature: std::extract to avoid copy
    const EventInfo smallestEvent = *m_events.begin();

    // Copy values from smallest event to out parameter
    traceEvent->CopyFrom(*smallestEvent.event);

    // Erase smallest event from set
    m_events.erase(m_events.begin());

    if (!m_readers[smallestEvent.queue]->isFinished()) {
        // Get next event into the set
        m_readers[smallestEvent.queue]->readTraceEvent(smallestEvent.event);

        // Insert new event and rebalance set
        m_events.insert(move(smallestEvent));
    }
}

bool TraceFileParser::isFinished() {
    for (const auto &reader : m_readers) {
        if (!reader->isFinished()) {
            return false;
        }
    }
    // All readers have finished
    return true;
}

}  // namespace octf
