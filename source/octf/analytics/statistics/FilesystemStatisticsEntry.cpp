/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/analytics/statistics/FilesystemStatisticsEntry.h>

namespace octf {

struct FilesystemStatisticsEntry::EntryKey {
    EntryKey()
            : name()
            , partId()
            , statsCase(StatisticsCase::NAME_NOT_SET) {}

    EntryKey(StatisticsCase sCase,
        const std::string &name,
        uint64_t partId)
            : name(name)
            , partId(partId)
            , statsCase(sCase) {}

    virtual ~EntryKey() {}

    EntryKey(const EntryKey &other)
            : name(other.name)
            , partId(other.partId)
            , statsCase(other.statsCase) {}

    EntryKey &operator=(const EntryKey &other) {
        if (this != &other) {
            name = other.name;
            partId = other.partId;
            statsCase = other.statsCase;
        }

        return *this;
    }

    bool operator==(const EntryKey &other) const {
        return name == other.name && partId == other.partId &&
        		statsCase == other.statsCase;
    }

    bool operator!=(const EntryKey &other) const {
        return !(*this == other);
    }

    bool operator<(const EntryKey &other) const {
        int result = name.compare(other.name);
        if (0 == result) {
            if (partId != other.partId) {
                return partId < other.partId;
            } else {
                return statsCase < other.statsCase;
            }
        }

        return result < 0;
    }

    std::string name;
    uint64_t partId;
    StatisticsCase statsCase;
};

FilesystemStatisticsEntry::FilesystemStatisticsEntry()
        : m_children()
        , m_ioStats()
        , m_partId()
        , m_statsCase(StatisticsCase::NAME_NOT_SET) {}

FilesystemStatisticsEntry::~FilesystemStatisticsEntry() {}

FilesystemStatisticsEntry::FilesystemStatisticsEntry(const FilesystemStatisticsEntry &other)
        : m_children(other.m_children)
        , m_ioStats(other.m_ioStats)
        , m_partId(other.m_partId)
        , m_statsCase(other.m_statsCase) {}

FilesystemStatisticsEntry &FilesystemStatisticsEntry::operator=(
        const FilesystemStatisticsEntry &other) {
    if (&other != this) {
        m_children = other.m_children;
        m_ioStats = other.m_ioStats;
        m_partId = other.m_partId;
        m_statsCase = other.m_statsCase;
    }

    return *this;
}

void FilesystemStatisticsEntry::fillProtoStatistics(
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

void FilesystemStatisticsEntry::fillProtoStatisticsEntry(
        proto::FilesystemStatisticsEntry *entry,
        const std::string &name) const {
    m_ioStats.getIoStatistics(entry->mutable_statistics());
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

void FilesystemStatisticsEntry::updateIoStats(
        const proto::trace::ParsedEvent &event) {
    m_ioStats.count(event);
}

void FilesystemStatisticsEntry::discard(const proto::trace::ParsedEvent &event) {
    if (m_partId == event.device().partition()) {
        m_ioStats.count(event);
    }

    for (auto &child : m_children) {
        child.second.discard(event);
    }
}

} /* namespace octf */
