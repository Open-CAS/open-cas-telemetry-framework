/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
#define SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H

#include <map>
#include <memory>
#include <queue>
#include <set>
#include <octf/fs/FileId.h>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

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
class ParsedIoTraceEventHandler
        : public TraceEventHandler<proto::trace::Event> {
public:
    ParsedIoTraceEventHandler(const std::string &tracePath);
    virtual ~ParsedIoTraceEventHandler();

    /**
     * @brief Handles parsed IO
     *
     * @param IO Parsed IO to be handle
     */
    virtual void handleIO(const proto::trace::ParsedEvent &io) = 0;

    void processEvents() override {
        TraceEventHandler<proto::trace::Event>::processEvents();
        flushEvents();
    }

    /**
     * @return Sum of all devices sizes in sectors
     */
    uint64_t getDevicesSize() const;

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
     * @brief Handles device description trace event
     *
     * @param devDesc Device description trace event
     */
    virtual void handleDeviceDescription(
            const proto::trace::EventDeviceDescription &devDesc) {
        (void) devDesc;
    }

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
    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }

    void handleEvent(std::shared_ptr<proto::trace::Event> traceEvent) override;

    virtual std::shared_ptr<proto::trace::Event> getEventMessagePrototype()
            override;

    void flushEvents();

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
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_PARSEDIOTRACEEVENTHANDLER_H
