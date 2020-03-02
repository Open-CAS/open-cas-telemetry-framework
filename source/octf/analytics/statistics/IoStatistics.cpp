/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/IoStatistics.h>
#include <octf/utils/Exception.h>

namespace octf {

struct IoStatistics::Stats {
    Stats(uint64_t lbaHitMapRangeSize)
            : sizeDistribution("sector", 4096, 2)
            , latencyDistribution("ns", 1, 2)
            , errors(0)
            , wc()
            , lbaHitMap()
            , lbaHitMapRangeSize(lbaHitMapRangeSize) {}

    Stats(const Stats &other)
            : sizeDistribution(other.sizeDistribution)
            , latencyDistribution(other.latencyDistribution)
            , errors(other.errors)
            , wc(other.wc)
            , lbaHitMap(other.lbaHitMap)
            , lbaHitMapRangeSize(other.lbaHitMapRangeSize) {}

    Stats &operator=(const Stats &other) {
        if (this != &other) {
            sizeDistribution = other.sizeDistribution;
            latencyDistribution = other.latencyDistribution;
            errors = other.errors;
            wc = other.wc;
        }

        return *this;
    }

    void getIoStatisticsEntry(proto::IoStatisticsEntry *entry,
                              uint64_t beginTime,
                              uint64_t endTime) const {
        sizeDistribution.getStatistics(entry->mutable_size());
        latencyDistribution.getStatistics(entry->mutable_latency());
        entry->set_errors(errors);

        double duration = endTime - beginTime;
        double durationS = duration / 1000.0 / 1000.0 / 1000.0;

        {
            // Set IOPS
            double count = sizeDistribution.getCount();
            double iops = durationS != 0.0 ? count / durationS : 0;

            auto &metric = (*entry->mutable_metrics())["throughput"];
            metric.set_unit("IOPS");
            metric.set_value(iops);
        }
        {
            // Set bandwidth in sectors
            double total = sizeDistribution.getTotal();
            // Convert to MiB
            total *= 512.0 / 1024.0 / 1024.0;
            double bandwidth = durationS != 0.0 ? total / durationS : 0;

            auto &metric = (*entry->mutable_metrics())["bandwidth"];
            metric.set_unit("MiB/s");
            metric.set_value(bandwidth);
        }
        {
            auto workset = wc.getWorkset();

            auto &metric = (*entry->mutable_metrics())["workset"];
            metric.set_unit("sector");
            metric.set_value(workset);
        }
    }

    void getLatencyHistogramEntry(proto::Histogram *entry) const {
        latencyDistribution.getHistogram(entry);
    }

    void getLbaHistogramEntry(proto::Histogram *entry) const {
        entry->set_unit("sector");

        for (auto &range : lbaHitMap) {
            auto protoRange = entry->add_range();
            protoRange->set_begin(range.first);
            protoRange->set_end(range.first + lbaHitMapRangeSize - 1);
            protoRange->set_count(range.second);
        }
    }

    Distribution sizeDistribution;
    Distribution latencyDistribution;
    uint64_t errors;
    WorksetCalculator wc;
    /**
     * Map of chunks with aggregated LBA hits. The key is the range start LBA
     */
    std::map<uint64_t, uint64_t> lbaHitMap;
    uint64_t lbaHitMapRangeSize;
};

IoStatistics::IoStatistics(uint64_t lbaHitMapRangeSize)
        : m_statistics(proto::trace::IoType_ARRAYSIZE,
                       Stats(lbaHitMapRangeSize))
        , m_total(new IoStatistics::Stats(lbaHitMapRangeSize))
        , m_flush(new IoStatistics::Stats(lbaHitMapRangeSize))
        , m_lbaHistRangeSize(lbaHitMapRangeSize)
        , m_startTime(0)
        , m_endTime(0)
        , m_lbaHistEnabled(false) {}

IoStatistics::IoStatistics(const IoStatistics &other)
        : m_statistics(other.m_statistics)
        , m_total(new Stats(*other.m_total))
        , m_flush(new Stats(*other.m_flush))
        , m_lbaHistRangeSize(other.m_lbaHistRangeSize)
        , m_startTime(other.m_startTime)
        , m_endTime(other.m_endTime)
        , m_lbaHistEnabled(other.m_lbaHistEnabled) {}

IoStatistics &IoStatistics::operator=(const IoStatistics &other) {
    if (this != &other) {
        m_statistics = other.m_statistics;
        *m_total = *other.m_total;
        *m_flush = *other.m_flush;
        m_lbaHistRangeSize = other.m_lbaHistRangeSize;
        m_startTime = other.m_startTime;
        m_endTime = other.m_endTime;
        m_lbaHistEnabled = other.m_lbaHistEnabled;
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

    // Update LBA hit maps
    if (m_lbaHistEnabled && len != 0) {
        // Beggining LBAs of ranges where io begins and ends
        uint64_t ioBeginRangeStart =
                (io.lba() / m_lbaHistRangeSize) * m_lbaHistRangeSize;
        uint64_t ioEndRangeStart =
                ((io.lba() + io.len() - 1) / m_lbaHistRangeSize) *
                m_lbaHistRangeSize;

        // How many ranges does this IO span
        uint64_t rangeSpan =
                (ioEndRangeStart - ioBeginRangeStart) / m_lbaHistRangeSize + 1;

        // Update each range
        for (uint64_t range = 0; range < rangeSpan; range++) {
            uint64_t rangeToUpdate =
                    range * m_lbaHistRangeSize + ioBeginRangeStart;
            stats->lbaHitMap[rangeToUpdate]++;
            m_total->lbaHitMap[rangeToUpdate]++;
        }
    }

    if (latency) {
        m_total->latencyDistribution += latency;
        stats->latencyDistribution += latency;
    }

    // Update error
    if (io.error()) {
        stats->errors++;
        m_total->errors++;
    }

    if (len) {
        // Update size
        m_total->sizeDistribution += len;
        stats->sizeDistribution += len;

        // update working set
        if (proto::trace::Discard == io.operation()) {
            for (auto &s : m_statistics) {
                s.wc.removeRange(io.lba(), len);
            }
            m_total->wc.removeRange(io.lba(), len);
        } else {
            stats->wc.insertRange(io.lba(), len);
            m_total->wc.insertRange(io.lba(), len);
        }
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

void octf::IoStatistics::getIoLatencyHistogram(
        proto::IoHistogram *histogram) const {
    auto read = histogram->mutable_read();
    m_statistics[proto::trace::IoType::Read].getLatencyHistogramEntry(read);

    auto write = histogram->mutable_write();
    m_statistics[proto::trace::IoType::Write].getLatencyHistogramEntry(write);

    auto discard = histogram->mutable_discard();
    m_statistics[proto::trace::IoType::Discard].getLatencyHistogramEntry(
            discard);

    auto flush = histogram->mutable_flush();
    m_flush->getLatencyHistogramEntry(flush);

    auto total = histogram->mutable_total();
    m_total->getLatencyHistogramEntry(total);

    histogram->set_duration(m_endTime - m_startTime);
}

void IoStatistics::getIoLbaHistogram(proto::IoHistogram *histogram) const {
    if (!m_lbaHistEnabled) {
        return;
    }

    auto discard = histogram->mutable_discard();
    m_statistics[proto::trace::IoType::Discard].getLbaHistogramEntry(discard);

    auto write = histogram->mutable_write();
    m_statistics[proto::trace::IoType::Write].getLbaHistogramEntry(write);

    auto read = histogram->mutable_read();
    m_statistics[proto::trace::IoType::Read].getLbaHistogramEntry(read);

    auto total = histogram->mutable_total();
    m_total->getLbaHistogramEntry(total);

    histogram->set_duration(m_endTime - m_startTime);
}

void IoStatistics::enableLbaHistogram() {
    m_lbaHistEnabled = true;
}

}  // namespace octf
