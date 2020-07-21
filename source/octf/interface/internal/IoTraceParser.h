/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H
#define SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H

#include <memory>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {
class IoTraceParser : public TraceEventHandler<proto::trace::Event> {
public:
    IoTraceParser(const std::string &tracePath)
            : TraceEventHandler<proto::trace::Event>(tracePath) {}

    virtual ~IoTraceParser() {}

    /*
     * @brief Skip IO's outside of this defined subrange
     * @param start LBA of subrange start
     * @param end LBA of subrange end
     *
     * @note any IO's which overlap with this range will also be included
     * @note when subrange is set, queue depth may be meaningless - use this
     *  only when queue depth is not considered - also that's why it's protected
     */
    virtual void setExclusiveSubrange(uint64_t start, uint64_t end) = 0;

    /**
     * Gets filesystem viewer interface
     *
     * This interface is used to inspect and view filesystem on the basis
     * of captured IO traces.
     *
     * @param partitionId
     *
     * @return
     */
    virtual IFileSystemViewer *getFileSystemViewer(uint64_t partitionId) = 0;

    /**
     * @return Sum of all devices sizes in sectors
     */
    virtual uint64_t getDevicesSize() const = 0;

    virtual std::shared_ptr<proto::trace::Event> getEventMessagePrototype() {
        return std::make_shared<proto::trace::Event>();
    }

    virtual void initParser() {
        getParser()->init();
    }

    virtual void deinitParser() {
        getParser()->deinit();
    }

protected:
    virtual bool compareEvents(const proto::trace::Event *a,
                               const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }
};
}  // namespace octf

#endif /* SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H */
