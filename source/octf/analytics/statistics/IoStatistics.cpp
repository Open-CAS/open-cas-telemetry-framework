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
            , LatencyDistribution("ns", 100, 10)
            , Errors(0) {}

    Stats(const Stats &other)
            : SizeDistribution(other.SizeDistribution)
            , LatencyDistribution(other.LatencyDistribution)
            , Errors(other.Errors) {}

    Stats &operator=(const Stats &other) {
        if (this != &other) {
            SizeDistribution = other.SizeDistribution;
            LatencyDistribution = other.LatencyDistribution;
            Errors = other.Errors;
        }

        return *this;
    }

    void getIoStatisticsEntry(proto::IoStatisticsEntry *entry) const {
        SizeDistribution.getDistribution(entry->mutable_size());
        LatencyDistribution.getDistribution(entry->mutable_latency());
        entry->set_errors(Errors);
    }

    Distribution SizeDistribution;
    Distribution LatencyDistribution;
    uint64_t Errors;
};

IoStatistics::IoStatistics()
        : m_statistics(proto::trace::IoType_ARRAYSIZE)
        , m_total(new IoStatistics::Stats())
        , m_flush(new IoStatistics::Stats())
        , m_invalid(new IoStatistics::Stats()) {}

IoStatistics::IoStatistics(const IoStatistics &other)
        : m_statistics(other.m_statistics)
        , m_total(new Stats(*other.m_total))
        , m_flush(new Stats(*other.m_flush))
        , m_invalid(new Stats(*other.m_invalid)) {}

IoStatistics &IoStatistics::operator=(const IoStatistics &other) {
    if (this != &other) {
        m_statistics = other.m_statistics;
        *m_total = *other.m_total;
        *m_flush = *other.m_flush;
        *m_invalid = *other.m_invalid;
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
    Stats *stats = &m_statistics[type];

    if (io.flush() && !io.len()) {
        // This IO is sync request, count it into flush IO group
        stats = m_flush.get();
    }

    auto len = io.len();
    auto latency = io.latency();

    if (latency) {
        m_total->SizeDistribution += len;
        m_total->LatencyDistribution += latency;
    } else {
        // Zero latency in nanoscend is impossible, treat it as an invalid IO
        stats = m_invalid.get();
        // TODO (mariuszbarczak) consider if print invalid IOs statistics
    }

    stats->SizeDistribution += len;
    stats->LatencyDistribution += latency;

    if (io.error()) {
        stats->Errors++;
    }
}

void IoStatistics::getIoStatistics(proto::IoStatistics *stats) const {
    auto read = stats->mutable_read();
    m_statistics[proto::trace::IoType::Read].getIoStatisticsEntry(read);

    auto write = stats->mutable_write();
    m_statistics[proto::trace::IoType::Write].getIoStatisticsEntry(write);

    auto discard = stats->mutable_discard();
    m_statistics[proto::trace::IoType::Discard].getIoStatisticsEntry(discard);

    auto flush = stats->mutable_flush();
    m_flush->getIoStatisticsEntry(flush);

    auto total = stats->mutable_total();
    m_total->getIoStatisticsEntry(total);
}

}  // namespace octf
