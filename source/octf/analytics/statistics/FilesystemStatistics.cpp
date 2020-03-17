/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/FilesystemStatistics.h>

namespace octf {

struct FilesystemStatistics::Key {
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

FilesystemStatistics::FilesystemStatistics()
        : m_map() {}

FilesystemStatistics::~FilesystemStatistics() {}

FilesystemStatistics::FilesystemStatistics(const FilesystemStatistics &other)
        : m_map(other.m_map) {}

FilesystemStatistics &FilesystemStatistics::operator=(
        const FilesystemStatistics &other) {
    if (&other != this) {
        m_map = other.m_map;
    }

    return *this;
}

void FilesystemStatistics::addDevice(
        const proto::trace::EventDeviceDescription &devDesc) {
    Key key(devDesc.id(), devDesc.size(), devDesc.name());

    // Check maybe key already exists
    auto iter = m_map.find(key);
    if (iter != m_map.end()) {
        // Key already exists, we have to update it, but need to keep the old
        // copy of IO statistics

        // Create pair of the updated key and the old statistics
        auto pair = std::make_pair(Key(key), FilesystemStatisticsEntry(iter->second));

        // Remove the old key
        m_map.erase(key);

        // Insert the new pair
        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot allocate IO statistics");
        }
    } else {
        // Key doesn't exist add it and allocate for them IO statistics
        getStatisticsByKey(key);
    }
}

void FilesystemStatistics::count(IFileSystemViewer *viewer,
                                 const proto::trace::ParsedEvent &event) {
    if (event.has_io()) {
        const auto &io = event.io();
        if (proto::trace::Discard == io.operation()) {
            discard(event);
            return;
        }
    }

    if (event.has_file()) {
        const auto &device = event.device();

        FileId id = FileId(event);
        Key key(device.id());

        getStatisticsByKey(key).updateIoStats(event);

        {
            // Update statistics by file extension
            auto ext = viewer->getFileExtension(id);
            if (ext != "") {
                getStatisticsByKey(key).updateIoStats(event);
            }
        }
        {
            // Update statistics by base name
            auto basename = viewer->getFileNamePrefix(id);
            if (basename != "") {
                getStatisticsByKey(key).updateIoStats(event);
            }
        }
    }
}

void FilesystemStatistics::discard(const proto::trace::ParsedEvent &event) {
    for (auto &child : m_map) {
        child.second.discard(event);
    }
}

void FilesystemStatistics::getFilesystemStatistics(
        proto::FilesystemStatistics *statistics) const {
	FilesystemStatisticsEntry entry;
	entry.fillProtoStatistics(statistics, "");
}

FilesystemStatisticsEntry &FilesystemStatistics::getStatisticsByKey(const Key &key) {
    auto iter = m_map.find(key);
    if (iter != m_map.end()) {
        auto pair = std::make_pair(Key(key), FilesystemStatisticsEntry());

        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot allocate FS statistics");
        }

        iter = result.first;
    }

    return iter->second;
}

} /* namespace octf */
