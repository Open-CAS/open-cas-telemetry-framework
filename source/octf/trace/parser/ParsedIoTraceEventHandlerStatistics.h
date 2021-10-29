/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H

#include <octf/analytics/statistics/IoStatisticsSet.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>

namespace octf {

class ParsedIoTraceEventHandlerStatistics : public ParsedIoTraceEventHandler {
public:
    ParsedIoTraceEventHandlerStatistics(
            const std::string &tracePath,
            uint64_t lbaHitRangeSize = DEFAULT_LBA_HIT_MAP_RANGE_SIZE)
            : ParsedIoTraceEventHandler(tracePath)
            , m_statisticsSet(lbaHitRangeSize) {}
    virtual ~ParsedIoTraceEventHandlerStatistics() = default;

    void handleIO(const proto::trace::ParsedEvent &io) override {
        m_statisticsSet.count(io);
    }

    const IoStatisticsSet &getStatisticsSet() const {
        return m_statisticsSet;
    }

    /**
     * @brief Enables creation of LBA histogram.
     * This needs to be enabled because keeping LBA histogram is expensive
     */
    void enableLbaHistogram() {
        m_statisticsSet.enableLbaHistogram();
    }

    /**
     * @brief Skip IO's outside of this defined subrange
     * @param start LBA of subrange start
     * @param end LBA of subrange end
     *
     * @note any IO's which overlap with this range will also be included
     */
    void setExclusiveSubrange(uint64_t start, uint64_t end) {
        ParsedIoTraceEventHandler::setExclusiveSubrange(start, end);
    }

    /** Default size of LBA hit map range in sectors == 10 MiB */
    static constexpr uint64_t DEFAULT_LBA_HIT_MAP_RANGE_SIZE = 20480;

protected:
    void handleDeviceDescription(
            const proto::trace::EventDeviceDescription &devDesc) override {
        m_statisticsSet.addDevice(devDesc);
    }

private:
    IoStatisticsSet m_statisticsSet;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLERSTATISTICS_H
