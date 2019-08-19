/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/IoStatisticsSet.h>

#include <string>

namespace octf {

struct IoStatisticsSet::Key {
    Key(uint64_t id, uint64_t size, const std::string &name)
            : Id(id)
            , Size(size)
            , Name(name) {}
    Key(uint64_t id)
            : Id(id)
            , Size(0)
            , Name("") {}
    Key()
            : Id()
            , Size()
            , Name("") {}
    virtual ~Key() {}

    Key(const Key &other)
            : Id(other.Id)
            , Size(other.Size)
            , Name(other.Name) {}

    Key &operator=(const Key &other) {
        if (this != &other) {
            Id = other.Id;
            Size = other.Size;
            Name = other.Name;
        }

        return *this;
    }

    bool operator==(const Key &other) const {
        return Id == other.Id;
    }

    bool operator!=(const Key &other) const {
        return Id != other.Id;
    }

    bool operator<(const Key &other) const {
        return Id < other.Id;
    }

    uint64_t Id;
    uint64_t Size;
    std::string Name;
};

IoStatisticsSet::IoStatisticsSet()
        : m_map() {}

IoStatisticsSet::~IoStatisticsSet() {}

void IoStatisticsSet::count(const proto::trace::ParsedEvent &event) {
    const auto &device = event.device();
    Key key(device.id());
    IoStatistics &stats = getIoStatistics(key);
    stats.count(event);
}

IoStatisticsSet::IoStatisticsSet(const IoStatisticsSet &other)
        : m_map(other.m_map) {}

IoStatisticsSet &IoStatisticsSet::operator=(const IoStatisticsSet &other) {
    if (this != &other) {
        m_map = other.m_map;
    }

    return *this;
}

void IoStatisticsSet::addDevice(
        const proto::trace::EventDeviceDescription &devDesc) {
    Key key(devDesc.id(), devDesc.size(), devDesc.name());

    // Check maybe key already exists
    auto iter = m_map.find(key);
    if (iter != m_map.end()) {
        // Key already exists, we have to update it, but need to keep the old
        // copy of IO statistics

        // Create pair of the updated key and the old statistics
        auto pair = std::make_pair(Key(key), IoStatistics(iter->second));

        // Remove the old key
        m_map.erase(key);

        // Insert the new pair
        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot allocate IO statistics");
        }
    } else {
        // Key doesn't exist add it and allocate for them IO statistics
        getIoStatistics(key);
    }
}

IoStatistics &IoStatisticsSet::getIoStatistics(const Key &key) {
    auto iter = m_map.find(key);
    if (iter == m_map.end()) {
        auto pair = std::make_pair(Key(key), IoStatistics());

        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot allocate IO statistics");
        }

        iter = result.first;
    }

    return iter->second;
}

void IoStatisticsSet::getIoStatisticsSet(proto::IoStatisticsSet *set) const {
    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_statistics();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);

        stats.second.getIoStatistics(dst);
    }
}

void IoStatisticsSet::getIoLatencyHistogramSet(proto::HistogramSet *set) const {
    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_histogram();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);

        stats.second.getIoLatencyHistogram(dst);
    }
}

}  // namespace octf
