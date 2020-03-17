/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_ENTRY_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_ENTRY_H

#include <map>
#include <string>
#include <octf/analytics/statistics/IoStatistics.h>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/statistics.pb.h>

namespace octf {

/**
 * @ingroup Statistics
 *
 * @brief Filesystem statistics breakdown by directories, file extensions, etc.
 *
 */
class FilesystemStatisticsEntry {
public:
    FilesystemStatisticsEntry();
    virtual ~FilesystemStatisticsEntry();
    FilesystemStatisticsEntry(FilesystemStatisticsEntry const &other);
    FilesystemStatisticsEntry &operator=(FilesystemStatisticsEntry const &other);

    static constexpr auto WIF_METRIC_NAME = "write invalidation factor";

    void updateIoStats(const proto::trace::ParsedEvent &event);

    struct EntryKey;

    void fillProtoStatistics(proto::FilesystemStatistics *statistics,
                             const std::string &dir) const;

    void fillProtoStatisticsEntry(proto::FilesystemStatisticsEntry *entry,
                                  const std::string &name) const;

    void discard(const proto::trace::ParsedEvent &event);

private:
    /**
     * Filesystem statistics for children directories
     */
    std::map<EntryKey, FilesystemStatisticsEntry> m_children;

    /**
     * IO statistics
     */
    IoStatistics m_ioStats;

    uint64_t m_partId;

    using StatisticsCase = proto::FilesystemStatisticsEntry::NameCase;
    /**
     * Statistics Case
     */
    StatisticsCase m_statsCase;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_ENTRY_H
