/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/interface/internal/IoTraceParser.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/v0/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/v1/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/v2/ParsedIoTraceEventHandler.h>

#include <octf/utils/Exception.h>

namespace octf {

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        const std::string &tracePath) {
    auto version =
            TraceLibrary::get().getTrace(tracePath)->getSummary().version();

    switch (version) {
    case 0:
        m_childParser = std::unique_ptr<trace::v0::ParsedIoTraceEventHandler>(
                new trace::v0::ParsedIoTraceEventHandler(this, tracePath));
        break;
    case 1:
        m_childParser = std::unique_ptr<trace::v1::ParsedIoTraceEventHandler>(
                new trace::v1::ParsedIoTraceEventHandler(this, tracePath));
        break;
    case 2:
        m_childParser = std::unique_ptr<trace::v2::ParsedIoTraceEventHandler>(
                new trace::v2::ParsedIoTraceEventHandler(this, tracePath));
        break;
    default:
        throw Exception("Trying to parse unrecognized trace version");
    }
}

ParsedIoTraceEventHandler::~ParsedIoTraceEventHandler() {}

void ParsedIoTraceEventHandler::processEvents() {
    m_childParser->processEvents();
}

void ParsedIoTraceEventHandler::cancel() {
    m_childParser->cancel();
}

bool ParsedIoTraceEventHandler::isCancelRequested() {
    return m_childParser->isCancelRequested();
}

void ParsedIoTraceEventHandler::setExclusiveSubrange(uint64_t start,
                                                     uint64_t end) {
    m_childParser->setExclusiveSubrange(start, end);
}

IFileSystemViewer *ParsedIoTraceEventHandler::getFileSystemViewer(
        uint64_t partitionId) {
    return m_childParser->getFileSystemViewer(partitionId);
}

uint64_t ParsedIoTraceEventHandler::getDevicesSize() const {
    return m_childParser->getDevicesSize();
}

uint64_t ParsedIoTraceEventHandler::getWorkingSetSize() const {
    return m_childParser->getWorkingSetSize();
}

void ParsedIoTraceEventHandler::reinit() {
    return m_childParser->reinit();
}

}  // namespace octf
