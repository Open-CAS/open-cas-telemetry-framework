/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H

#include <octf/analytics/statistics/FilesystemStatistics.h>
#include <octf/proto/statistics.pb.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>

namespace octf {

/**
 * This handler provides filesystem statistics.
 *
 * The workload traces are breakdown by directories, files extensions, etc.
 */
class TraceEventHandlerFilesystemStatistics : public ParsedIoTraceEventHandler {
public:
    TraceEventHandlerFilesystemStatistics(const std::string &tracePath);
    virtual ~TraceEventHandlerFilesystemStatistics();

    virtual void handleIO(const octf::proto::trace::ParsedEvent &io) override;

    /**
     * @brief Gets computed protocol buffer filesystem statistics
     *
     * @param[out] fsStats Filesystem statistics in protocol buffer format
     */
    void getFilesystemStatistics(proto::FilesystemStatistics *fsStats) const;

private:
    FilesystemStatistics m_fsStats;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H
