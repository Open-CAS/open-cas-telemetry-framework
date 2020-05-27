/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_V0_PARSEDIOTRACEEVENTHANDLER_H
#define SOURCE_OCTF_TRACE_PARSER_V0_PARSEDIOTRACEEVENTHANDLER_H

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <octf/fs/FileId.h>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/interface/IIoTraceParser.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

namespace v0 {
/**
 * This is IO trace event handler of parsed IO
 *
 * The parsed IO is created from multiple events (especially IO request and IO
 * completion). It contains basic IO information (LBA, length, etc). It is
 * supplemented by related information like filesystem one. In addition it
 * provides post parse information (latency, queue depth, etc...).
 *
 * @note The order of handled IO respect the IOs queuing order
 */
class ParsedIoTraceEventHandler : public IIoTraceParser {
public:
    ParsedIoTraceEventHandler(octf::ParsedIoTraceEventHandler *parentHandler);
    virtual ~ParsedIoTraceEventHandler();

    /**
     * @return Sum of all devices sizes in sectors
     */
    uint64_t getDevicesSize() const;

    void flushEvents();

protected:
    /**
     * Gets filesystem viewer interface
     *
     * This interface is used to inspect and view filesystem on the basis
     * of captured IO traces.
     *
     * @param deviceID
     *
     * @return
     */
    IFileSystemViewer *getFileSystemViewer(uint64_t partitionID);

    /**
     * @brief Skip IO's outside of this defined subrange
     * @param start LBA of subrange start
     * @param end LBA of subrange end
     *
     * @note any IO's which overlap with this range will also be included
     * @note when subrange is set, queue depth may be meaningless - use this
     *  only when queue depth is not considered - also that's why it's protected
     */
    void setExclusiveSubrange(uint64_t start, uint64_t end);

private:
    void handleEvent(std::shared_ptr<proto::trace::Event> traceEvent) override;

    void pushOutEvent();

private:
    struct Key;
    class Map;
    struct IoQueueDepth;
    class FileSystemViewer;
    struct FileInfo;
    std::queue<proto::trace::ParsedEvent> m_queue;
    std::unique_ptr<Map> m_eventMapping;
    std::map<uint64_t, proto::trace::ParsedEvent *> m_sidMapping;
    std::map<uint64_t, proto::trace::EventDeviceDescription> m_devices;
    std::map<FileId, FileInfo> m_fileInfo;
    uint64_t m_timestampOffset;
    uint64_t m_sidOffset;
    uint64_t m_limit;
    uint64_t m_subrangeStart;
    uint64_t m_subrangeEnd;
    std::map<uint64_t, IoQueueDepth> m_devIoQueueDepth;
    std::map<uint64_t, FileSystemViewer> m_partitionFsViewers;

    octf::ParsedIoTraceEventHandler *m_parentHandler;
};

}  // namespace v0

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_V0_PARSEDIOTRACEEVENTHANDLER_H
