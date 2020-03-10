/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/FilesystemStatistics.h>

namespace octf {

struct FilesystemStatistics::Key {
    Key()
            : name()
            , devId()
            , partId()
            , statsCase(StatisticsCase::NAME_NOT_SET) {}

    Key(StatisticsCase sCase,
        const std::string &name,
        uint64_t devId,
        uint64_t partId)
            : name(name)
            , devId(devId)
            , partId(partId)
            , statsCase(sCase) {}

    virtual ~Key() {}

    Key(const Key &other)
            : name(other.name)
            , devId(other.devId)
            , partId(other.partId)
            , statsCase(other.statsCase) {}

    Key &operator=(const Key &other) {
        if (this != &other) {
            name = other.name;
            devId = other.devId;
            partId = other.partId;
            statsCase = other.statsCase;
        }

        return *this;
    }

    bool operator==(const Key &other) const {
        return name == other.name && devId == other.devId &&
               partId == other.partId && statsCase == other.statsCase;
    }

    bool operator!=(const Key &other) const {
        return !(*this == other);
    }

    bool operator<(const Key &other) const {
        int result = name.compare(other.name);
        if (0 == result) {
            if (devId != other.devId) {
                return devId < other.devId;
            } else if (partId != other.partId) {
                return partId < other.partId;
            } else {
                return statsCase < other.statsCase;
            }
        }

        return result < 0;
    }

    std::string name;
    uint64_t devId;
    uint64_t partId;
    StatisticsCase statsCase;
};

FilesystemStatistics::FilesystemStatistics()
        : m_children()
        , m_ioStats()
        , m_devId()
        , m_partId()
        , m_statsCase(StatisticsCase::NAME_NOT_SET) {}

FilesystemStatistics::~FilesystemStatistics() {}

FilesystemStatistics::FilesystemStatistics(const FilesystemStatistics &other)
        : m_children(other.m_children)
        , m_ioStats(other.m_ioStats)
        , m_devId(other.m_devId)
        , m_partId(other.m_partId)
        , m_statsCase(other.m_statsCase) {}

FilesystemStatistics &FilesystemStatistics::operator=(
        const FilesystemStatistics &other) {
    if (&other != this) {
        m_children = other.m_children;
        m_ioStats = other.m_ioStats;
        m_devId = other.m_devId;
        m_partId = other.m_partId;
        m_statsCase = other.m_statsCase;
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
        const auto &device = event.device();

        FileId id = FileId(event);

        auto &statistics =
                getStatisticsByIds(viewer, viewer->getParentId(id),
                                   device.id());
        statistics.updateIoStats(event);

        {
            // Update statistics by file extension
            auto ext = viewer->getFileExtension(id);
            if (ext != "") {
                Key key(StatisticsCase::kFileExtension, ext, device.id(),
                        device.partition());
                getStatisticsByKey(key).updateIoStats(event);
            }
        }
        {
            // Update statistics by base name
            auto basename = viewer->getFileNamePrefix(id);
            if (basename != "") {
                Key key(StatisticsCase::kFileNamePrefix, basename, device.id(),
                        device.partition());
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
        const FileId &dirId,
        uint64_t devId) {
    FilesystemStatistics *statistics = NULL;
    FileId parentId = viewer->getParentId(dirId);

    if (parentId == dirId) {
        statistics = this;
    } else {
        statistics = &getStatisticsByIds(viewer, parentId, devId);
    }

    std::string name = viewer->getFileName(dirId);
    Key key(StatisticsCase::kDirectory, name, devId, dirId.partitionId);

    return statistics->getStatisticsByKey(key);
}

void FilesystemStatistics::fillProtoStatistics(
        proto::FilesystemStatistics *statistics,
        const std::string &dir) const {
    proto::FilesystemStatisticsEntry entry;

    if (dir != "") {
        fillProtoStatisticsEntry(&entry, dir);

        if (m_statsCase != StatisticsCase::kDirectory) {
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
        auto statsCase = child.first.statsCase;

        if (name.empty()) {
            continue;
        }

        if (statsCase == StatisticsCase::kDirectory) {
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
        proto::FilesystemStatisticsEntry *entry,
        const std::string &name) const {
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

    switch (m_statsCase) {
    case StatisticsCase::kDirectory:
        entry->set_directory(name);
        break;
    case StatisticsCase::kFileNamePrefix:
        entry->set_filenameprefix(name);
        break;
    case StatisticsCase::kFileExtension:
        entry->set_fileextension(name);
        break;
    default:
        throw Exception("Invalid statistics case");
        break;
    }
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
    newFsStats.m_statsCase = key.statsCase;

    return newFsStats;
}

} /* namespace octf */
