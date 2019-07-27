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
            , Size()
            , Power()
            , Begin()
            , End()
            , RangeSize()
            , Sum()
            , Count() {}

    Bucket(const Bucket &other)
            : Level(other.Level)
            , Size(other.Size)
            , Power(other.Power)
            , Begin(other.Begin)
            , End(other.End)
            , RangeSize(other.RangeSize)
            , Sum(other.Sum)
            , Count(other.Count) {}

    Bucket &operator=(const Bucket &other) {
        if (&other != this) {
            Level = other.Level;
            Size = other.Size;
            Power = other.Power;
            Begin = other.Begin;
            End = other.End;
            RangeSize = other.RangeSize;
            Sum = other.Sum;
            Count = other.Count;
        }

        return *this;
    }

    Bucket(uint64_t level, uint64_t size, uint64_t power)
            : Sum(size)
            , Count(size) {
        Level = level;
        Size = size;
        Power = power;
        Begin = 0;
        if (level) {
            Begin = std::pow(power, level - 1) * size;
        }
        End = std::pow(power, level) * size - 1;

        // Calculate particular range size in backet
        RangeSize = (End - Begin + 1) / size;
    }

    void operator+=(uint64_t value) {
        if (value < Begin || value > End) {
            throw Exception("Updating wrong distribution bucket");
        }

        uint64_t rangeId = value;
        rangeId -= Begin;
        rangeId /= RangeSize;

        if (rangeId < Size) {
            Sum[rangeId] += value;
            Count[rangeId]++;
        } else {
            throw Exception("Updating distribution bucket fatal error");
        }
    }

    uint64_t Level;
    uint64_t Size;
    uint64_t Power;
    uint64_t Begin;
    uint64_t End;
    uint64_t RangeSize;

    std::vector<uint64_t> Sum;
    std::vector<uint64_t> Count;
};

Distribution::Distribution(const std::string &unit,
                           uint64_t bucketSize,
                           uint64_t bucketPower)
        : m_unit(unit)
        , m_bucketSize(bucketSize)
        , m_bucketPower(bucketPower)
        , m_total()
        , m_count()
        , m_min(std::numeric_limits<decltype(m_min)>::max())
        , m_max()
        , m_histogram() {
    if (bucketPower < 2) {
        throw Exception("Invalid bucket power");
    }
}

Distribution::Distribution(const Distribution &other)
        : m_unit(other.m_unit)
        , m_bucketSize(other.m_bucketSize)
        , m_bucketPower(other.m_bucketPower)
        , m_total(other.m_total)
        , m_count(other.m_count)
        , m_min(other.m_min)
        , m_max(other.m_max)
        , m_histogram(other.m_histogram) {}

Distribution &Distribution::operator=(const Distribution &other) {
    if (this != &other) {
        m_unit = other.m_unit;
        m_bucketSize = other.m_bucketSize;
        m_bucketPower = other.m_bucketPower;
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
    uint64_t size = m_bucketSize;

    while (size <= value) {
        size *= m_bucketPower;
        level++;
    }

    if (level >= m_histogram.size()) {
        uint64_t j = m_histogram.size();
        m_histogram.resize(level + 1);

        // Initialized missed buckets
        for (; j <= level; j++) {
            m_histogram[j] = Bucket(j, m_bucketSize, m_bucketPower);
        }
    }

    return m_histogram[level];
}

}  // namespace octf
