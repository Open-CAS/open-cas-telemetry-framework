/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H

#include <map>
#include <string>
#include <octf/analytics/statistics/IoStatistics.h>
#include <octf/analytics/statistics/FilesystemStatisticsEntry.h>
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
     * @brief Add devices to the FS statistics
     *
     * @param devDesc Device description trace event
     */
    void addDevice(const proto::trace::EventDeviceDescription &devDesc);

    /**
     * Get Filesystem statistics in protocol buffer format
     *
     * @param[out] statistics Protocol buffer filesystem statistics
     */
    void getFilesystemStatistics(proto::FilesystemStatistics *statistics) const;

    static constexpr auto WIF_METRIC_NAME = "write invalidation factor";

private:
    struct Key;

    FilesystemStatisticsEntry &getStatisticsByKey(const Key &key);

    void discard(const proto::trace::ParsedEvent &event);

private:
    /**
     * Filesystem statistics for children directories
     */
    std::map<Key, FilesystemStatisticsEntry> m_map;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_FILESYSTEMSTATISTICS_H
