/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/FilesystemStatistics.h>

namespace octf {

static constexpr auto WIF_METRIC_NAME = "write invalidation factor";

struct FilesystemStatistics::Key {
    Key()
            : name()
            , devId()
            , partId() {}

    Key(const std::string &name, uint64_t devId, uint64_t partId)
            : name(name)
            , devId(devId)
            , partId(partId) {}

    virtual ~Key() {}

    Key(const Key &other)
            : name(other.name)
            , devId(other.devId)
            , partId(other.partId) {}

    Key &operator=(const Key &other) {
        if (this != &other) {
            name = other.name;
            devId = other.devId;
            partId = other.partId;
        }

        return *this;
    }

    bool operator==(const Key &other) const {
        return name == other.name && devId == other.devId &&
               partId == other.partId;
    }

    bool operator!=(const Key &other) const {
        return !(*this == other);
    }

    bool operator<(const Key &other) const {
        int result = name.compare(other.name);
        if (0 == result) {
            if (devId != other.devId) {
                return devId < other.devId;
            }
            return partId < other.partId;
        }

        return result < 0;
    }

    std::string name;
    uint64_t devId;
    uint64_t partId;
};

FilesystemStatistics::FilesystemStatistics()
        : m_children()
        , m_ioStats()
        , m_devId()
        , m_partId() {}

FilesystemStatistics::~FilesystemStatistics() {}

FilesystemStatistics::FilesystemStatistics(const FilesystemStatistics &other)
        : m_children(other.m_children)
        , m_ioStats(other.m_ioStats)
        , m_devId(other.m_devId)
        , m_partId(other.m_partId) {}

FilesystemStatistics &FilesystemStatistics::operator=(
        const FilesystemStatistics &other) {
    if (&other != this) {
        m_children = other.m_children;
        m_ioStats = other.m_ioStats;
        m_devId = other.m_devId;
        m_partId = other.m_partId;
    }

    return *this;
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
        const auto &file = event.file();
        const auto &device = event.device();
        auto id = file.id();

        auto &statistics = getStatisticsByIds(viewer, viewer->getParentId(id),
                                              device.id(), device.partition());
        statistics.updateIoStats(event);

        {
            // Update statistics by file extension
            auto ext = viewer->getFileExtension(id);
            if (ext != "") {
                Key key("*." + ext, device.id(), device.partition());
                getStatisticsByKey(key).updateIoStats(event);
            }
        }
        {
            // Update statistics by base name
            auto basename = viewer->getBaseName(id);
            if (basename != "") {
                Key key(basename + ".*", device.id(), device.partition());
                getStatisticsByKey(key).updateIoStats(event);
            }
        }
    }
}

void FilesystemStatistics::getFilesystemStatistics(
        proto::FilesystemStatistics *statistics) const {
    fillProtoStatistics(statistics, "");
}

FilesystemStatistics &FilesystemStatistics::getStatisticsByIds(
        IFileSystemViewer *viewer,
        uint64_t dirId,
        uint64_t devId,
        uint64_t partId) {
    FilesystemStatistics *statistics = NULL;
    uint64_t parentId = viewer->getParentId(dirId);

    if (parentId == dirId) {
        statistics = this;
    } else {
        statistics = &getStatisticsByIds(viewer, parentId, devId, partId);
    }

    std::string name = viewer->getFileName(dirId);
    Key key(name, devId, partId);

    return statistics->getStatisticsByKey(key);
}

void FilesystemStatistics::fillProtoStatistics(
        proto::FilesystemStatistics *statistics,
        const std::string &dir) const {
    bool checkWif = false;
    proto::FilesystemStatisticsEntry entry;

    fillProtoStatisticsEntry(&entry);

    if (dir != "") {
        if (dir.front() == '*') {
            // Extension case
            entry.set_extension(dir);
            checkWif = true;
        } else if (dir.back() == '*') {
            // Basename case
            entry.set_basename(dir);
            checkWif = true;
        } else {
            entry.set_directory(dir);
        }

        if (checkWif) {
            auto &metrics = *entry.mutable_statistics()
                                     ->mutable_write()
                                     ->mutable_metrics();

            if (metrics[WIF_METRIC_NAME].value() > 1.0L) {
                // For non-directory stats, add only statistics which have write
                // invalidation factor greater than 1.0. It minimize output
                // size.
                statistics->add_entries()->CopyFrom(entry);
            }
        } else {
            statistics->add_entries()->CopyFrom(entry);
        }
    }

    for (const auto &child : m_children) {
        std::string name = child.first.name;

        if (name.empty()) {
            continue;
        }

        if (name.front() != '*' && name.back() != '*') {
            // Directory case
            if (name.back() != '/' && !dir.empty() && dir.back() != '/') {
                name = dir + "/" + name;
            } else {
                name = dir + name;
            }
        }

        child.second.fillProtoStatistics(statistics, name);
    }
}

void FilesystemStatistics::fillProtoStatisticsEntry(
        proto::FilesystemStatisticsEntry *entry) const {
    m_ioStats.getIoStatistics(entry->mutable_statistics());
    entry->set_deviceid(m_devId);
    entry->set_partitionid(m_partId);

    // Clear proto statistics
    auto pStats = entry->mutable_statistics();
    pStats->clear_flush();
    pStats->clear_discard();
    pStats->mutable_read()->clear_latency();
    pStats->mutable_write()->clear_latency();
    pStats->mutable_total()->clear_latency();
    pStats->mutable_read()->mutable_size()->clear_percentiles();
    pStats->mutable_write()->mutable_size()->clear_percentiles();
    pStats->mutable_total()->mutable_size()->clear_percentiles();

    // Calculate WIF and insert metric
    auto metrics = pStats->mutable_write()->mutable_metrics();
    double workset = (*metrics)["workset"].value();
    double written =
            pStats->write().size().total() + pStats->discard().size().total();

    double wif = 0.0L;
    if (workset != 0.0L) {
        wif = written / workset;
    }

    (*metrics)[WIF_METRIC_NAME].set_value(wif);
}

void FilesystemStatistics::updateIoStats(
        const proto::trace::ParsedEvent &event) {
    m_ioStats.count(event);
}

void FilesystemStatistics::discard(const proto::trace::ParsedEvent &event) {
    if (m_devId == event.device().id()) {
        m_ioStats.count(event);
    }

    for (auto &child : m_children) {
        child.second.discard(event);
    }
}

FilesystemStatistics &FilesystemStatistics::getStatisticsByKey(const Key &key) {
    auto iter = m_children.find(key);
    if (iter != m_children.end()) {
        return iter->second;
    }

    auto &newFsStats = m_children[key];

    newFsStats.m_devId = key.devId;
    newFsStats.m_partId = key.partId;

    return newFsStats;
}

} /* namespace octf */
