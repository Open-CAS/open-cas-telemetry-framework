/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/IoStatisticsSet.h>

#include <string>

namespace octf {

struct IoStatisticsSet::Key {
    Key(uint64_t id, uint64_t size = 0, const std::string &name = "")
            : Id(id)
            , Size(size)
            , Name(name) {}

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
        return Size == other.Size;
    }

    bool operator!=(const Key &other) const {
        return Size != other.Size;
    }

    bool operator<(const Key &other) const {
        return Size < other.Size;
    }

    uint64_t Id;
    uint64_t Size;
    std::string Name;
};

IoStatisticsSet::IoStatisticsSet()
        : NonCopyable()
        , m_map() {}

IoStatisticsSet::~IoStatisticsSet() {}

void IoStatisticsSet::count(const proto::trace::ParsedEvent &event) {
    const auto &device = event.device();
    Key key(device.id(), 0, device.name());
    IoStatistics &stats = getIoStatistics(key);
    stats.count(event);
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

void IoStatisticsSet::fillIoStatisticsSet(proto::IoStatisticsSet *set) const {
    for (const auto &stats : m_map) {
        auto dst = set->add_statistics();

        auto device = dst->mutable_desc()->mutable_devcie();
        device->set_id(stats.first.Id);
        device->set_name(stats.first.Name);
        device->set_size(stats.first.Size);

        stats.second.fillIoStatistics(dst);
    }
}

}  // namespace octf
