/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H

#include <memory>
#include <octf/analytics/statistics/FilesystemStatistics.h>
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
     * @brief Gets computed filesystem statistics
     *
     * @return Filesystem statistics
     */
    const FilesystemStatistics &getFilesystemStatistics(void) const;

private:
    std::unique_ptr<FilesystemStatistics> m_fsStats;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERFILESYSTEMSTATISTICS_H
