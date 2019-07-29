/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/Distribution.h>

#include <cmath>
#include <limits>
#include <octf/utils/Exception.h>

namespace octf {

struct Distribution::Bucket {
    Bucket()
            : Level()
            , RangesCount()
            , Begin()
            , End()
            , RangeSize()
            , Sum()
            , Count() {}

    Bucket(const Bucket &other)
            : Level(other.Level)
            , RangesCount(other.RangesCount)
            , Begin(other.Begin)
            , End(other.End)
            , RangeSize(other.RangeSize)
            , Sum(other.Sum)
            , Count(other.Count) {}

    Bucket &operator=(const Bucket &other) {
        if (&other != this) {
            Level = other.Level;
            RangesCount = other.RangesCount;
            Begin = other.Begin;
            End = other.End;
            RangeSize = other.RangeSize;
            Sum = other.Sum;
            Count = other.Count;
        }

        return *this;
    }

    Bucket(uint64_t level, uint64_t ranges, uint64_t base)
            : Sum(ranges)
            , Count(ranges) {
        Level = level;
        RangesCount = ranges;
        Begin = 0;
        if (level) {
            Begin = std::pow(base, level - 1) * ranges;
        }
        End = std::pow(base, level) * ranges - 1;

        // Calculate particular range size in bucket
        RangeSize = (End - Begin + 1) / ranges;
    }

    void operator+=(uint64_t value) {
        if (value < Begin || value > End) {
            throw Exception("Updating wrong distribution bucket");
        }

        uint64_t rangeId = value;
        rangeId -= Begin;
        rangeId /= RangeSize;

        if (rangeId < RangesCount) {
            Sum[rangeId] += value;
            Count[rangeId]++;
        } else {
            throw Exception("Updating distribution bucket fatal error");
        }
    }

    uint64_t Level;
    uint64_t RangesCount;
    uint64_t Begin;
    uint64_t End;
    uint64_t RangeSize;

    std::vector<uint64_t> Sum;
    std::vector<uint64_t> Count;
};

Distribution::Distribution(const std::string &unit,
                           uint64_t ranges,
                           uint64_t base)
        : m_unit(unit)
        , m_ranges(ranges)
        , m_base(base)
        , m_total()
        , m_count()
        , m_min(std::numeric_limits<decltype(m_min)>::max())
        , m_max()
        , m_histogram() {
    if (base < 2) {
        throw Exception("Invalid bucket base");
    }
}

Distribution::Distribution(const Distribution &other)
        : m_unit(other.m_unit)
        , m_ranges(other.m_ranges)
        , m_base(other.m_base)
        , m_total(other.m_total)
        , m_count(other.m_count)
        , m_min(other.m_min)
        , m_max(other.m_max)
        , m_histogram(other.m_histogram) {}

Distribution &Distribution::operator=(const Distribution &other) {
    if (this != &other) {
        m_unit = other.m_unit;
        m_ranges = other.m_ranges;
        m_base = other.m_base;
        m_total = other.m_total;
        m_count = other.m_count;
        m_min = other.m_min;
        m_max = other.m_max;
        m_histogram = other.m_histogram;
    }

    return *this;
}

Distribution::~Distribution() {}

void Distribution::operator+=(uint64_t value) {
    m_total += value;
    m_count++;
    m_min = std::min(m_min, value);
    m_max = std::max(m_max, value);

    // Update histogram
    auto &bucket = getBucket(value);
    bucket += value;
}

void Distribution::getDistribution(proto::Distribution *distribution) const {
    distribution->Clear();
    distribution->set_avarage(m_count ? m_total / m_count : 0);
    distribution->set_min(m_count ? m_min : 0);
    distribution->set_max(m_max);
    distribution->set_count(m_count);
    distribution->set_total(m_total);
    distribution->set_unit(m_unit);
}

Distribution::Bucket &Distribution::getBucket(uint64_t value) {
    uint64_t level = 0;
    uint64_t end = m_ranges - 1;
    uint64_t base = 1;

    while (end < value) {
        level++;
        base *= m_base;
        end = base * m_ranges - 1;
    }

    if (level >= m_histogram.size()) {
        uint64_t j = m_histogram.size();
        m_histogram.resize(level + 1);

        // Initialized missed buckets
        for (; j <= level; j++) {
            m_histogram[j] = Bucket(j, m_ranges, m_base);
        }
    }

    return m_histogram[level];
}

}  // namespace octf
