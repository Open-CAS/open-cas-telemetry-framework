/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/IoStatisticsSet.h>

#include <string>

namespace octf {

struct IoStatisticsSet::Key {
    Key(uint64_t id,
        uint64_t size,
        const std::string &name,
        const std::string &model)
            : Id(id)
            , Size(size)
            , Name(name)
            , Model(model) {}
    Key(uint64_t id)
            : Id(id)
            , Size(0)
            , Name("")
            , Model("") {}
    Key()
            : Id()
            , Size()
            , Name("")
            , Model("") {}
    virtual ~Key() {}

    Key(const Key &other)
            : Id(other.Id)
            , Size(other.Size)
            , Name(other.Name)
            , Model(other.Model) {}

    Key &operator=(const Key &other) {
        if (this != &other) {
            Id = other.Id;
            Size = other.Size;
            Name = other.Name;
            Model = other.Model;
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
    std::string Model;
};

IoStatisticsSet::IoStatisticsSet(uint64_t lbaHitRangeSize)
        : m_map()
        , m_lbaHitRangeSize(lbaHitRangeSize)
        , m_lbaHistEnabled(false) {}

IoStatisticsSet::~IoStatisticsSet() {}

void IoStatisticsSet::count(const proto::trace::ParsedEvent &event) {
    const auto &device = event.device();
    Key key(device.id());
    IoStatistics &stats = getIoStatistics(key);
    stats.count(event);
}

IoStatisticsSet::IoStatisticsSet(const IoStatisticsSet &other)
        : m_map(other.m_map)
        , m_lbaHitRangeSize(other.m_lbaHitRangeSize)
        , m_lbaHistEnabled(other.m_lbaHistEnabled) {}

IoStatisticsSet &IoStatisticsSet::operator=(const IoStatisticsSet &other) {
    if (this != &other) {
        m_map = other.m_map;
        m_lbaHitRangeSize = other.m_lbaHitRangeSize;
        m_lbaHistEnabled = other.m_lbaHistEnabled;
    }

    return *this;
}

void IoStatisticsSet::addDevice(
        const proto::trace::EventDeviceDescription &devDesc) {
    Key key(devDesc.id(), devDesc.size(), devDesc.name(), devDesc.model());

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

        if (m_lbaHistEnabled) {
            result.first->second.enableLbaHistogram();
        }
    } else {
        // Key doesn't exist add it and allocate for them IO statistics
        getIoStatistics(key);
    }
}

IoStatistics &IoStatisticsSet::getIoStatistics(const Key &key) {
    auto iter = m_map.find(key);
    if (iter == m_map.end()) {
        auto pair = std::make_pair(Key(key), IoStatistics(m_lbaHitRangeSize));

        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot allocate IO statistics");
        }

        iter = result.first;
    }

    return iter->second;
}

void IoStatisticsSet::getIoStatisticsSet(proto::IoStatisticsSet *set) const {
    // Allocate statistics then it will be printed in JSON
    set->mutable_statistics();

    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_statistics();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);
        device->set_model(stats.first.Model);

        stats.second.getIoStatistics(dst);
    }
}

void IoStatisticsSet::getIoLatencyHistogramSet(
        proto::IoHistogramSet *set) const {
    // Allocate histogram then it will be printed in JSON
    set->mutable_histogram();

    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_histogram();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);
        device->set_model(stats.first.Model);

        stats.second.getIoLatencyHistogram(dst);
    }
}

void IoStatisticsSet::getIoLbaHistogramSet(proto::IoHistogramSet *set) const {
    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_histogram();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);
        device->set_model(stats.first.Model);

        stats.second.getIoLbaHistogram(dst);
    }
}

void IoStatisticsSet::getIoSizeHistogramSet(proto::IoHistogramSet *set) const {
    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_histogram();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);
        device->set_model(stats.first.Model);

        stats.second.getIoSizeHistogram(dst);
    }
}

void IoStatisticsSet::getQueueDepthHistogramSet(
        proto::IoHistogramSet *set) const {
    // For each pair in map
    for (const auto &stats : m_map) {
        auto dst = set->add_histogram();

        auto device = dst->mutable_desc()->mutable_device();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);
        device->set_model(stats.first.Model);

        stats.second.getQueueDepthHistogram(dst);
    }
}

void IoStatisticsSet::enableLbaHistogram() {
    m_lbaHistEnabled = true;
}

}  // namespace octf
