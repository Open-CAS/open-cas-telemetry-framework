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
 * The set is grouped by devices
 */
class IoStatisticsSet {
public:
    IoStatisticsSet(uint64_t lbaHitRangeSize);
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
     * @brief Add devices to the IO statistics
     *
     * @param devDesc Device description trace event
     */
    void addDevice(const proto::trace::EventDeviceDescription &devDesc);

    /**
     * @brief Copies gathered statistics of IOs into protocol buffer IO
     * statistics set object
     *
     * @param[out] set protocol buffer IO statistics object to be filled
     */
    void getIoStatisticsSet(proto::IoStatisticsSet *set) const;

    /**
     * @brief Copies gathered IO latency statistics into protocol buffer
     * histogram set object
     *
     * @param[out] set protocol buffer IO histogram object to be filled
     */
    void getIoLatencyHistogramSet(proto::IoHistogramSet *set) const;

    /**
     * @brief Copies gathered IO LBA hits statistics into protocol buffer
     * histogram set object
     *
     * @param[out] set protocol buffer IO histogram object to be filled
     */
    void getIoLbaHistogramSet(proto::IoHistogramSet *set) const;

    /**
     * @brief Enables creation of LBA histogram.
     * This needs to be enabled because keeping LBA histogram is expensive
     */
    void enableLbaHistogram();

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

    /**
     * @brief Size of range, in which lba hits are aggregated
     */
    uint64_t m_lbaHitRangeSize;

    /**
     * @brief This flag indicates if computing of LBA histogram is enabled
     */
    bool m_lbaHistEnabled;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICSSET_H
