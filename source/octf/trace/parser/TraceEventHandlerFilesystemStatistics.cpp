/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/parser/TraceEventHandlerFilesystemStatistics.h>

namespace octf {

TraceEventHandlerFilesystemStatistics::TraceEventHandlerFilesystemStatistics(
        const std::string &tracePath)
        : ParsedIoTraceEventHandler(tracePath)
        , m_fsStats() {}

TraceEventHandlerFilesystemStatistics::
        ~TraceEventHandlerFilesystemStatistics() {}

void TraceEventHandlerFilesystemStatistics::handleIO(
        const octf::proto::trace::ParsedEvent &io) {
    IFileSystemViewer *viewer = getFileSystemViewer(io.device().partition());
    m_fsStats.count(viewer, io);
}

void TraceEventHandlerFilesystemStatistics::getFilesystemStatistics(
        proto::FilesystemStatistics *fsStats) const {
    m_fsStats.getFilesystemStatistics(fsStats);
}

void TraceEventHandlerFilesystemStatistics::handleDeviceDescription(
        const proto::trace::EventDeviceDescription &devDesc) {
	m_fsStats.addDevice(devDesc);
}

}  // namespace octf
