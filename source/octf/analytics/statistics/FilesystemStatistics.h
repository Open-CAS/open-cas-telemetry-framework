/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H

#include <map>
#include <string>
#include <octf/analytics/statistics/IFileSystemViewer.h>
#include <octf/analytics/statistics/IoStatistics.h>
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

private:
    FilesystemStatistics &get(IFileSystemViewer *viewer,
                              uint64_t id,
                              const proto::trace::ParsedEvent &event);

    void fill(proto::FilesystemStatistics *statistics,
              const std::string &dir) const;

    void fill(proto::FilesystemStatisticsEntry *entry,
              const IoStatistics &ioStats) const;

    void count(const proto::trace::ParsedEvent &event);

    void discard(const proto::trace::ParsedEvent &event);

private:
    /**
     * Filesystem statistics for children directories
     */
    std::map<std::string, FilesystemStatistics> m_children;

    /**
     * IO statistics per partition
     */
    std::map<uint64_t, IoStatistics> m_ioStats;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
