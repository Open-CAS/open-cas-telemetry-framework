/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/v0/ParsedIoTraceEventHandler.h>

#include <octf/utils/Exception.h>

namespace octf {

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        const std::string &tracePath)
        : TraceEventHandler<proto::trace::Event>(tracePath) {
    auto majorVersion =
            TraceLibrary::get().getTrace(tracePath)->getSummary().version();

    switch (majorVersion) {
    case 0:
        m_childParser = std::unique_ptr<v0::ParsedIoTraceEventHandler>(
                new v0::ParsedIoTraceEventHandler(this));
        break;
    default:
        throw Exception("Trying to parse unrecognized trace version");
    }
}

ParsedIoTraceEventHandler::~ParsedIoTraceEventHandler() {}

void ParsedIoTraceEventHandler::handleEvent(
        std::shared_ptr<proto::trace::Event> traceEvent) {
    m_childParser->handleEvent(traceEvent);
}

std::shared_ptr<proto::trace::Event>
ParsedIoTraceEventHandler::getEventMessagePrototype() {
    return std::make_shared<proto::trace::Event>();
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

}  // namespace octf
