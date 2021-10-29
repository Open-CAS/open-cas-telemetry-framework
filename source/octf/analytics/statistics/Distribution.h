/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H

#include <map>
#include <string>
#include <vector>
#include <octf/proto/statistics.pb.h>
#include <octf/utils/Exception.h>

namespace octf {

/**
 * @ingroup Statistics
 * @brief Distribution of values
 */
class Distribution {
public:
    /**
     * @brief Distribution of values
     *
     * The distribution provides histogram. It is hard to track each value
     * because of memory consumption. Thus distribution bucketizes the values.
     * The bucket sizes are defined by two variables:
     * 1. Ranges - Number of ranges in bucket. Each bucket contains the same
     * number of ranges. Each range in bucket has the same size.
     * 2. Base - Value of base used to compute a next bucket.
     *
     * The following equation defines the beginning and the end of buckets:
     * Begin = 0 for level 0, otherwise: (base ^ (level - 1)) * ranges
     * End = (base ^ level) * ranges - 1
     *
     * An example of histogram for following buckets configuration:
     * ranges = 10
     * base = 10
     *
     * +--------------+------------+------------+-----+------------+
     * | Bucket level |  Range 0   |  Range 1   | ... | Range 9    |
     * +--------------+------------+------------+-----+------------+
     * |            0 | [0..0]     | [1..1]     | ... | [9..9]     |
     * |            1 | [10..18]   | [19..27]   | ... | [91..99]   |
     * |            2 | [100..189] | [190..279] | ... | [910..999] |
     * |          ... | ...        | ...        | ... | ...        |
     * +--------------+------------+------------+-----+------------+
     *
     * @param unit Unit for values of this distribution
     * @param ranges Number of ranges in bucket
     * @param base Value of base used to compute a next bucket
     */
    Distribution(const std::string &unit, uint64_t ranges, uint64_t base);
    Distribution(Distribution const &other);
    Distribution &operator=(Distribution const &other);
    virtual ~Distribution();

    /**
     * @brief Count value
     *
     * @param value Value to count in distribution
     */
    void operator+=(uint64_t value);

    /**
     * @brief Copies distribution's values into protocol buffer object
     *
     * @param[out] statistics Protocol buffer statistics values object to be
     * filled
     */
    void getStatistics(proto::StatisticsEntryValues *statistics) const;

    /**
     * @brief Copies distribution histogram into protocol buffer histogram
     * object
     *
     * @param[out] statistics Protocol buffer histogram object to be filled
     */
    void getHistogram(proto::Histogram *histogram) const;

    /**
     * Gets count of items in this distribution
     *
     * @return Count of items in this distribution
     */
    uint64_t getCount() const {
        return m_count;
    }

    /**
     * Gets total sum of items in this distribution
     *
     * @return Total sum of items in this distribution
     */
    uint64_t getTotal() const {
        return m_total;
    }

private:
    struct Bucket;
    Bucket &getBucket(uint64_t value);

private:
    /**
     * Unit for values of this distribution
     */
    std::string m_unit;

    /**
     * Bucket size
     */
    uint64_t m_ranges;

    /**
     * Bucket base used to calculate a next level bucket
     */
    uint64_t m_base;

    /**
     * Total sum of items in this distribution
     */
    uint64_t m_total;

    /**
     * Count of items in this distribution
     */
    uint64_t m_count;

    /**
     * Minimum value of distribution's items
     */
    uint64_t m_min;

    /**
     * Maximum value of distribution's items
     */
    uint64_t m_max;

    /**
     * Bucketized histogram of values
     */
    std::vector<Bucket> m_histogram;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H
