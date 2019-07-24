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

class Distribution {
public:
    Distribution(const std::string &unit,
                 uint64_t bucketSize,
                 uint64_t backetPower);
    Distribution(Distribution const &other);
    Distribution &operator=(Distribution const &other);

    virtual ~Distribution();

    void operator+=(uint64_t value);

    void fillDistribution(proto::Distribution *distribution) const;

private:
    struct Backet;

    Backet &getBucket(uint64_t value);

private:
    std::string m_unit;
    uint64_t m_bucketSize;
    uint64_t m_bucketPower;
    uint64_t m_total;
    uint64_t m_count;
    uint64_t m_min;
    uint64_t m_max;
    std::vector<Backet> m_histogram;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_DISTRIBUTION_H
