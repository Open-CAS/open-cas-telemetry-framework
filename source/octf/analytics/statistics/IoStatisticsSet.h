/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICSSET_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICSSET_H

#include <set>
#include <unordered_map>
#include <octf/analytics/statistics/IoStatistics.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/statistics.pb.h>

namespace octf {

/**
 * @defgroup Statistics Statistics
 * Statistics utilities
 */

/**
 * @ingroup Statistics
 * @brief A utility class which provides set of statistics for IO events.
 *
 * The set is grouped by device
 */
class IoStatisticsSet {
public:
    IoStatisticsSet();
    IoStatisticsSet(IoStatisticsSet const &other);
    IoStatisticsSet &operator=(IoStatisticsSet const &other);
    virtual ~IoStatisticsSet();

    /**
     * @brief Counts IO and updates statistics
     *
     * @param event parsed IO event
     */
    void count(const proto::trace::ParsedEvent &event);

    /**
     * @brief Copies gathers statistics of IOs into protocol buffer IO
     * statistics set object
     *
     * @param[out] set protocol buffer IO statistics object to be filled
     */
    void fillIoStatisticsSet(proto::IoStatisticsSet *set) const;

private:
    struct Key;
    /**
     * @brief Map of IO Statistics
     *
     * It can be keyed by a devices, an IO classes etc...
     */
    std::map<Key, IoStatistics> m_map;

private:
    /**
     * @brief Gets IO Statistics for a specified key
     * @return IO Statistics for a specified key
     */
    IoStatistics &getIoStatistics(const Key &key);
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICSSET_H
