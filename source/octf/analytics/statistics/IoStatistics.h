/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H

#include <vector>
#include <octf/analytics/statistics/Distribution.h>
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
    IoStatistics();
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
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H
