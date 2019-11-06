/*
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/analytics/statistics/FilesystemStatistics.h>

namespace octf {

static constexpr auto WIF_METRIC_NAME = "write invalidation factor";

FilesystemStatistics::FilesystemStatistics()
        : m_children()
        , m_ioStats() {}

FilesystemStatistics::~FilesystemStatistics() {}

FilesystemStatistics::FilesystemStatistics(const FilesystemStatistics &other)
        : m_children(other.m_children)
        , m_ioStats(other.m_ioStats) {}

FilesystemStatistics &FilesystemStatistics::operator=(
        const FilesystemStatistics &other) {
    if (&other != this) {
        m_children = other.m_children;
        m_ioStats = other.m_ioStats;
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
        auto id = file.id();

        auto &statistics = get(viewer, viewer->getParentId(id), event);
        statistics.count(event);

        {
            // Update statistics by file extension
            auto ext = viewer->getFileExtension(id);
            if (ext != "") {
                m_children["*." + ext].count(event);
            }
        }
        {
            // Update statistics by base name
            auto basename = viewer->getBaseName(id);
            if (basename != "") {
                m_children[basename + ".*"].count(event);
            }
        }
    }
}

void FilesystemStatistics::getFilesystemStatistics(
        proto::FilesystemStatistics *statistics) const {
    fill(statistics, "");
}

FilesystemStatistics &FilesystemStatistics::get(
        IFileSystemViewer *viewer,
        uint64_t id,
        const proto::trace::ParsedEvent &event) {
    FilesystemStatistics *statistics = NULL;
    uint64_t parentId = viewer->getParentId(id);

    if (parentId == id) {
        statistics = this;
    } else {
        statistics = &get(viewer, parentId, event);
    }

    std::string name = viewer->getFileName(id);
    return statistics->m_children[name];
}

void FilesystemStatistics::fill(proto::FilesystemStatistics *statistics,
                                const std::string &dir) const {
    for (const auto &part : m_ioStats) {
        bool checkWif = false;
        proto::FilesystemStatisticsEntry entry;

        entry.set_partitionid(part.first);
        fill(&entry, part.second);

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

            if (metrics[WIF_METRIC_NAME].value() <= 1.0) {
                continue;
            }
        }

        statistics->add_entries()->CopyFrom(entry);
    }

    for (const auto &child : m_children) {
        std::string name = child.first;

        if (name.front() != '*' && name.back() != '*') {
            // Directory case
            if (name.back() != '/' && !dir.empty() && dir.back() != '/') {
                name = dir + "/" + name;
            } else {
                name = dir + name;
            }
        }

        child.second.fill(statistics, name);
    }
}

void FilesystemStatistics::fill(proto::FilesystemStatisticsEntry *entry,
                                const IoStatistics &ioStats) const {
    ioStats.getIoStatistics(entry->mutable_statistics());

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

    double wif = written / workset;
    (*metrics)[WIF_METRIC_NAME].set_value(wif);
}

void FilesystemStatistics::count(const proto::trace::ParsedEvent &event) {
    auto partId = event.file().partitionid();
    m_ioStats[partId].count(event);
}

void FilesystemStatistics::discard(const proto::trace::ParsedEvent &event) {
    for (auto &part : m_ioStats) {
        part.second.count(event);
    }

    for (auto &child : m_children) {
        child.second.discard(event);
    }
}

} /* namespace octf */
