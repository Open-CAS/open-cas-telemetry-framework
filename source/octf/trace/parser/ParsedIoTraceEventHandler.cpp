/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/v4/ParsedIoTraceEventHandler.h>

#include <octf/utils/Exception.h>

namespace octf {

ParsedIoTraceEventHandler::ParsedIoTraceEventHandler(
        const std::string &tracePath)
        : TraceEventHandler<proto::trace::Event>(tracePath) {
    auto majorVersion =
            TraceLibrary::get().getTrace(tracePath)->getSummary().major();

    switch (majorVersion) {
    case 0:
    case 4:
        m_childParser = std::make_shared<v4::ParsedIoTraceEventHandler>(this);
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
