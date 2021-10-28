/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H

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
class FilesystemStatistics {
public:
    FilesystemStatistics();
    virtual ~FilesystemStatistics();
    FilesystemStatistics(FilesystemStatistics const &other);
    FilesystemStatistics &operator=(FilesystemStatistics const &other);

    /**
     * @brief Counts IO and updates filesystem statistics
     *
     * @param viewer Filesystem viewer
     * @param event parsed IO event
     */
    void count(IFileSystemViewer *viewer,
               const proto::trace::ParsedEvent &event);

    /**
     * Get Filesystem statistics in protocol buffer format
     *
     * @param[out] statistics Protocol buffer filesystem statistics
     */
    void getFilesystemStatistics(proto::FilesystemStatistics *statistics) const;

    static constexpr auto WIF_METRIC_NAME = "write invalidation factor";

private:
    struct Key;

    FilesystemStatistics &getStatisticsByKey(const Key &key);

    FilesystemStatistics &getStatisticsByIds(IFileSystemViewer *viewer,
                                             const FileId &dirId,
                                             uint64_t devId);

    void fillProtoStatistics(proto::FilesystemStatistics *statistics,
                             const std::string &dir) const;

    void fillProtoStatisticsEntry(proto::FilesystemStatisticsEntry *entry,
                                  const std::string &name) const;

    void updateIoStats(const proto::trace::ParsedEvent &event);

    void discard(const proto::trace::ParsedEvent &event);

private:
    /**
     * Filesystem statistics for children directories
     */
    std::map<Key, FilesystemStatistics> m_children;

    /**
     * IO statistics
     */
    IoStatistics m_ioStats;

    uint64_t m_devId;
    uint64_t m_partId;

    using StatisticsCase = proto::FilesystemStatisticsEntry::NameCase;
    /**
     * Statistics Case
     */
    StatisticsCase m_statsCase;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
