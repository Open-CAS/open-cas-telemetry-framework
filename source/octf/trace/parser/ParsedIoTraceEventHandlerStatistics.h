/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H

#include <octf/analytics/statistics/IoStatisticsSet.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>

namespace octf {

class ParsedIoTraceEventHandlerStatistics : public ParsedIoTraceEventHandler {
public:
    ParsedIoTraceEventHandlerStatistics(const std::string &tracePath)
            : ParsedIoTraceEventHandler(tracePath)
            , m_statisticsSet() {}
    virtual ~ParsedIoTraceEventHandlerStatistics() = default;

    void handleIO(const proto::trace::ParsedEvent &io) {
        m_statisticsSet.count(io);
    }

    const IoStatisticsSet &getStatisticsSet() const {
        return m_statisticsSet;
    }

private:
    IoStatisticsSet m_statisticsSet;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H
