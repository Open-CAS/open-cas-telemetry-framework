/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/parser/TraceEventHandlerFilesystemStatistics.h>

namespace octf {

TraceEventHandlerFilesystemStatistics::TraceEventHandlerFilesystemStatistics(
        const std::string &tracePath)
        : ParsedIoTraceEventHandler(tracePath)
        , m_fsStats(new FilesystemStatistics()) {}

TraceEventHandlerFilesystemStatistics::
        ~TraceEventHandlerFilesystemStatistics() {}

void TraceEventHandlerFilesystemStatistics::handleIO(
        const octf::proto::trace::ParsedEvent &io) {
    IFileSystemViewer *viewer = getFileSystemViewer(io.device().id());
    m_fsStats->count(viewer, io);
}

const FilesystemStatistics &
TraceEventHandlerFilesystemStatistics::getFilesystemStatistics(void) const {
    return *m_fsStats;
}

}  // namespace octf
