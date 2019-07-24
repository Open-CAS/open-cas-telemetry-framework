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

class IoStatistics {
public:
    IoStatistics();
    IoStatistics(IoStatistics const &other);
    IoStatistics &operator=(IoStatistics const &other);
    virtual ~IoStatistics();

    void count(const proto::trace::ParsedEvent &event);

    void fillIoStatistics(proto::IoStatistics *stats) const;

private:
    struct Stats;
    std::vector<Stats> m_statistics;
    std::unique_ptr<Stats> m_total;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICS_H
