/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/Distribution.h>

#include <cmath>
#include <limits>

namespace octf {

struct Distribution::Backet {
    Backet()
            : Id()
            , Size()
            , Power()
            , Begin()
            , End()
            , Step()
            , Sum()
            , Count() {}

    Backet(const Backet &other)
            : Id(other.Id)
            , Size(other.Size)
            , Power(other.Power)
            , Begin(other.Begin)
            , End(other.End)
            , Step(other.Step)
            , Sum(other.Sum)
            , Count(other.Count) {}

    Backet &operator=(const Backet &other) {
        if (&other != this) {
            Id = other.Id;
            Size = other.Size;
            Power = other.Power;
            Begin = other.Begin;
            End = other.End;
            Step = other.Step;
            Sum = other.Sum;
            Count = other.Count;
        }

        return *this;
    }

    Backet(uint64_t id, uint64_t size, uint64_t power)
            : Sum(size)
            , Count(size) {
        Id = id;
        Size = size;
        Power = power;
        Begin = 0;
        if (id) {
            Begin = std::pow(power, id - 1) * size;
        }
        End = std::pow(power, id) * size - 1;
        Step = (End - Begin + 1) / size;
    }

    void operator+=(uint64_t value) {
        if (value < Begin || value > End) {
            throw Exception("Updating wrong distribution bucket");
        }

        uint id = value;
        id -= Begin;
        id /= Step;

        if (id < Size) {
            Sum[id] += value;
            Count[id]++;
        } else {
            throw Exception("Updating distribution bucket fatal error");
        }
    }

    uint64_t Id;
    uint64_t Size;
    uint64_t Power;
    uint64_t Begin;
    uint64_t End;
    uint64_t Step;

    std::vector<uint64_t> Sum;
    std::vector<uint64_t> Count;
};

Distribution::Distribution(const std::string &unit,
                           uint64_t bucketSize,
                           uint64_t backetPower)
        : m_unit(unit)
        , m_bucketSize(bucketSize)
        , m_bucketPower(backetPower)
        , m_total()
        , m_count()
        , m_min(std::numeric_limits<decltype(m_min)>::max())
        , m_max()
        , m_histogram() {
    if (backetPower < 2) {
        // XXX
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
    auto &backet = getBucket(value);
    backet += value;
}

void Distribution::fillDistribution(proto::Distribution *distribution) const {
    distribution->Clear();
    distribution->set_avarage(m_count ? m_total / m_count : 0);
    distribution->set_min(m_count ? m_min : 0);
    distribution->set_max(m_max);
    distribution->set_count(m_count);
    distribution->set_total(m_total);
    distribution->set_unit(m_unit);

    // XXX
    return;

    for (const auto &bucket : m_histogram) {
        uint64_t begin = bucket.Begin;
        uint64_t end = begin + bucket.Step;

        for (uint64_t i = 0; i < bucket.Size; i++) {
            if (bucket.Count[i]) {
                std::string name =
                        std::to_string(begin) + ".." + std::to_string(end);

                (*distribution->mutable_histogram())[name] = bucket.Count[i];
            }

            begin += bucket.Step;
            end += bucket.Step;
        }
    }
}

Distribution::Backet &Distribution::getBucket(uint64_t value) {
    uint64_t i = 0;
    uint64_t size = m_bucketSize;

    while (size <= value) {
        size *= m_bucketPower;
        i++;
    }

    if (i >= m_histogram.size()) {
        uint64_t j = m_histogram.size();
        m_histogram.resize(i + 1);
        for (; j <= i; j++) {
            m_histogram[j] = Backet(j, m_bucketSize, m_bucketPower);
        }
    }

    return m_histogram[i];
}

}  // namespace octf
