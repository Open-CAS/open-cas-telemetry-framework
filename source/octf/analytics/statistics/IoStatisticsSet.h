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
#include <octf/utils/NonCopyable.h>

namespace octf {

class IoStatisticsSet : public NonCopyable {
public:
    IoStatisticsSet();
    virtual ~IoStatisticsSet();

    void count(const proto::trace::ParsedEvent &event);

    void fillIoStatisticsSet(proto::IoStatisticsSet *set) const;

private:
    struct Key;
    std::map<Key, IoStatistics> m_map;

private:
    IoStatistics &getIoStatistics(const Key &key);
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IOSTATISTICSSET_H
