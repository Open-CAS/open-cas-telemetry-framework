/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/IoStatistics.h>
#include <octf/utils/Exception.h>

namespace octf {

struct IoStatistics::Stats {
    Stats()
            : SizeDistribution("sector", 4096, 2)
            , LatencyDistribution("ns", 100, 10) {}

    Stats(const Stats &other)
            : SizeDistribution(other.SizeDistribution)
            , LatencyDistribution(other.LatencyDistribution) {}

    Stats &operator=(const Stats &other) {
        if (this != &other) {
            SizeDistribution = other.SizeDistribution;
            LatencyDistribution = other.LatencyDistribution;
        }

        return *this;
    }

    void fillIoStatisticsEntry(proto::IoStatisticsEntry *entry) const {
        SizeDistribution.fillDistribution(entry->mutable_size());
        LatencyDistribution.fillDistribution(entry->mutable_latency());
    }

    Distribution SizeDistribution;
    Distribution LatencyDistribution;
};

IoStatistics::IoStatistics()
        : m_statistics(proto::trace::IoType_ARRAYSIZE)
        , m_total(new IoStatistics::Stats()) {}

IoStatistics::IoStatistics(const IoStatistics &other)
        : m_statistics(other.m_statistics)
        , m_total(new Stats(*other.m_total)) {}

IoStatistics &IoStatistics::operator=(const IoStatistics &other) {
    if (this != &other) {
        m_statistics = other.m_statistics;
    }

    return *this;
}

IoStatistics::~IoStatistics() {}

void IoStatistics::count(const proto::trace::ParsedEvent &event) {
    const auto &io = event.io();
    proto::trace::IoType type = io.operation();

    int size = m_statistics.size();
    if (size < type) {
        throw Exception("Invalid IO type");
    }
    Stats &stats = m_statistics[type];

    // TODO (mbarczak) consider flush IOs

    auto len = io.len();
    auto latency = io.latency();

    stats.SizeDistribution += len;
    stats.LatencyDistribution += latency;

    (*m_total).SizeDistribution += len;
    (*m_total).LatencyDistribution += latency;
}

void IoStatistics::fillIoStatistics(proto::IoStatistics *stats) const {
    auto read = stats->mutable_read();
    m_statistics[proto::trace::IoType::Read].fillIoStatisticsEntry(read);

    auto write = stats->mutable_write();
    m_statistics[proto::trace::IoType::Write].fillIoStatisticsEntry(write);

    auto discard = stats->mutable_discard();
    m_statistics[proto::trace::IoType::Discard].fillIoStatisticsEntry(discard);

    auto total = stats->mutable_total();
    m_total->fillIoStatisticsEntry(total);
}

}  // namespace octf
