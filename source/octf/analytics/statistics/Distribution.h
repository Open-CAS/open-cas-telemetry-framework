/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H

#include <cmath>
#include <map>
#include <vector>
#include <octf/proto/statistics.pb.h>
#include <octf/utils/Exception.h>

namespace octf {

class Distribution {
public:
    Distribution(uint64_t bucketSize, uint64_t backetPower);

    virtual ~Distribution();

    void operator+=(uint64_t value);

    void getDistribution(proto::Distribution &distribution);

private:
    struct Backet;

    Backet &getBucket(uint64_t value);

private:
    const uint64_t m_bucketSize;
    const uint64_t m_backetPower;

    uint64_t m_sum;
    uint64_t m_count;
    uint64_t m_min;
    uint64_t m_max;
    std::vector<Backet> m_histogram;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H
