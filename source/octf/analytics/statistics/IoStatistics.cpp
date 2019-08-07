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
            , Errors(0)
            , Wc() {}

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

    void getIoStatisticsEntry(proto::IoStatisticsEntry *entry,
                              uint64_t beginTime,
                              uint64_t endTime) const {
        SizeDistribution.getDistribution(entry->mutable_size());
        LatencyDistribution.getDistribution(entry->mutable_latency());
        entry->set_errors(Errors);

        double duration = endTime - beginTime;
        double durationS = duration / 1000.0 / 1000.0 / 1000.0;

        {
            // Set IOPS
            double count = SizeDistribution.getCount();
            double iops = durationS != 0.0 ? count / durationS : 0;
            if (iops != 0.0) {
                auto metric = entry->add_metrics();
                metric->set_name("throughput");
                metric->set_unit("IOPS");
                metric->set_value(iops);
            }
        }
        {
            // Set bandwidth in sectors
            double total = SizeDistribution.getTotal();
            // Convert to MiB
            total *= 512.0 / 1024.0 / 1024.0;
            double bandwidth = durationS != 0.0 ? total / durationS : 0;

            if (bandwidth) {
                auto metric = entry->add_metrics();
                metric->set_name("bandwidth");
                metric->set_unit("MiB/s");
                metric->set_value(bandwidth);
            }
        }
        {
            uint64_t workset = Wc.getWorkset();
            auto metric = entry->add_metrics();
            metric->set_name("workset");
            metric->set_unit("sector");
            metric->set_value(workset);
        }
    }

    Distribution SizeDistribution;
    Distribution LatencyDistribution;
    uint64_t Errors;
    WorksetCalculator Wc;
};

IoStatistics::IoStatistics()
        : m_statistics(proto::trace::IoType_ARRAYSIZE)
        , m_total(new IoStatistics::Stats())
        , m_flush(new IoStatistics::Stats())
        , m_invalid(new IoStatistics::Stats())
        , m_startTime(0)
        , m_endTime(0) {}

IoStatistics::IoStatistics(const IoStatistics &other)
        : m_statistics(other.m_statistics)
        , m_total(new Stats(*other.m_total))
        , m_flush(new Stats(*other.m_flush))
        , m_invalid(new Stats(*other.m_invalid))
        , m_startTime(other.m_startTime)
        , m_endTime(other.m_endTime) {}

IoStatistics &IoStatistics::operator=(const IoStatistics &other) {
    if (this != &other) {
        m_statistics = other.m_statistics;
        *m_total = *other.m_total;
        *m_flush = *other.m_flush;
        *m_invalid = *other.m_invalid;
        m_startTime = other.m_startTime;
        m_endTime = other.m_endTime;
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
    stats->Wc.insertRange(io.lba(), len);

    if (io.error()) {
        stats->Errors++;
    }

    // Update time
    if (!m_startTime) {
        m_startTime = event.header().timestamp();
    }
    m_endTime = event.header().timestamp();
}

void IoStatistics::getIoStatistics(proto::IoStatistics *stats) const {
    auto read = stats->mutable_read();
    m_statistics[proto::trace::IoType::Read].getIoStatisticsEntry(
            read, m_startTime, m_endTime);

    auto write = stats->mutable_write();
    m_statistics[proto::trace::IoType::Write].getIoStatisticsEntry(
            write, m_startTime, m_endTime);

    auto discard = stats->mutable_discard();
    m_statistics[proto::trace::IoType::Discard].getIoStatisticsEntry(
            discard, m_startTime, m_endTime);

    auto flush = stats->mutable_flush();
    m_flush->getIoStatisticsEntry(flush, m_startTime, m_endTime);

    auto total = stats->mutable_total();
    m_total->getIoStatisticsEntry(total, m_startTime, m_endTime);

    stats->set_duration(m_endTime - m_startTime);
}

}  // namespace octf
