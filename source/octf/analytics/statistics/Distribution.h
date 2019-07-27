/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
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
     * The distribution provides histogram. It is hard track each value because
     * memory consumption. Thus distribution bucketize the values. The buckets
     * ranges are specified in constructor.
     *
     * The example of histogram for following buckets configuration:
     * bucketSize = 10
     * bucketPower = 2
     *
     * +--------------+------------+------------+-----+----------------------+
     * | Bucket level |  Range 1   |  Range 2   | ... | Last Range in Bucket |
     * +--------------+------------+------------+-----+----------------------+
     * |            0 | [0..1]     | [2..3]     | ... | [8..9]               |
     * |            2 | [10..19]   | [20..29]   | ... | [90..99]             |
     * |            3 | [100..199] | [200..199] | ... | [900..999]           |
     * |          ... | ...        | ...        | ... | ...                  |
     * +--------------+------------+------------+-----+----------------------+
     *
     * @param unit Unit for values of this distribution
     * @param bucketSize
     * @param backetPower
     */
    Distribution(const std::string &unit,
                 uint64_t bucketSize,
                 uint64_t backetPower);
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
     * @brief Copies distribution of values into protocol buffer distribution
     * object
     *
     * @param[out] distribution Protocol buffer distribution object to be filled
     */
    void getDistribution(proto::Distribution *distribution) const;

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
    uint64_t m_bucketSize;

    /**
     * Bucket power for next level
     */
    uint64_t m_bucketPower;

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
