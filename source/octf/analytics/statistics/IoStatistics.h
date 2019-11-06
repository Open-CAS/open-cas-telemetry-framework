/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H

#include <vector>
#include <octf/analytics/statistics/Distribution.h>
#include <octf/analytics/statistics/WorksetCalculator.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/statistics.pb.h>

namespace octf {

/**
 * @ingroup Statistics
 *
 * @brief A utility class which provides statistics for IO events
 *
 * The IO statistics shall contain values for a single device
 */
class IoStatistics {
public:
    static constexpr uint64_t LBA_HIT_MAP_RANGE_SIZE_DEFAULT = 20480;

    /**
     * @param lbaHitMapRangeSize Size in sectors of range in which LBA hits are
     * aggregated
     */
    IoStatistics(uint64_t lbaHitMapRangeSize = LBA_HIT_MAP_RANGE_SIZE_DEFAULT);
    IoStatistics(IoStatistics const &other);
    IoStatistics &operator=(IoStatistics const &other);
    virtual ~IoStatistics();

    /**
     * @brief Counts IO and updates statistics
     *
     * @param event parsed IO event
     */
    void count(const proto::trace::ParsedEvent &event);

    /**
     * @brief Copies gathers statistics of IOs into protocol buffer IO
     * statistics object
     *
     * @param[out] stats protocol buffer IO statistics object to be filled
     */
    void getIoStatistics(proto::IoStatistics *stats) const;

    /**
     * @brief Copies gathered statistics of IOs latency into protocol buffer IO
     * histogram object
     *
     * @param[out] histogram protocol buffer histogram object to be filled
     */
    void getIoLatencyHistogram(proto::IoHistogram *histogram) const;

    /**
     * @brief Copies gathered statistics of IOs LBA hits into protocol buffer IO
     * histogram object
     *
     * @param[out] histogram protocol buffer histogram object to be filled
     */
    void getIoLbaHistogram(proto::IoHistogram *histogram) const;

    /**
     * @brief Enables creation of LBA histogram.
     * This needs to be enabled because keeping LBA histogram is expensive
     */
    void enableLbaHistogram();

private:
    struct Stats;
    /**
     * @brief IO statistics grouped by operation type (read, write, discard)
     * excluding flush IOs
     */
    std::vector<Stats> m_statistics;

    /**
     * @brief Total IO statistics summary of each operation type
     */
    std::unique_ptr<Stats> m_total;

    /**
     * @brief IO statistics summary for flush IOs
     */
    std::unique_ptr<Stats> m_flush;

    /**
     * @brief IO statistics summary for invalid IOs, e.g. an IO with zero
     * latency
     */
    std::unique_ptr<Stats> m_invalid;

    /**
     * @brief Size of range, in which lba hits are aggregated
     */
    uint64_t m_lbaHistRangeSize;

    /**
     * @brief Start time
     */
    uint64_t m_startTime;

    /**
     * @brief End time
     */
    uint64_t m_endTime;

    bool m_lbaHistEnabled;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H
