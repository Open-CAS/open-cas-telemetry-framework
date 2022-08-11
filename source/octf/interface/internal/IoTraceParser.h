/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H
#define SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H

#include <memory>
#include <octf/fs/IFileSystemViewer.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/ITrace.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/TraceEventHandler.h>
#include <octf/trace/parser/TraceEventHandlerWorkset.h>

namespace octf {
class IoTraceParser : public TraceEventHandler<proto::trace::Event> {
public:
    IoTraceParser(const std::string &tracePath)
            : TraceEventHandler<proto::trace::Event>(tracePath)
            , m_trace(TraceLibrary::get().getTrace(tracePath)) {}

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

    /**
     * @brief Gets the trace path of parsed trace
     *
     * @return Trace path of parsed trace
     */
    virtual std::string const &getTracePath() const {
        return m_trace->getPath();
    };

    /**
     * @brief Gets the trace reference of parsed trace
     *
     * @return Trace reference of parsed trace
     */
    virtual TraceShRef getTrace() const {
        return m_trace;
    };

    /**
     * @brief Gets the working set size of the trace
     *
     * @return uint64_t Working set size
     */
    virtual uint64_t getWorkingSetSize() const {
        auto trace = getTrace();
        auto &cache = trace->getCache();
        uint64_t workset = 0;

        if (!cache.read("WorkingSetSize", workset)) {
            CasTraceEventHandlerWorkset handler(trace->getPath());

            handler.processEvents();
            workset = handler.getWorkset();
            cache.write("WorkingSetSize", workset);
        }

        return workset;
    }

    virtual std::shared_ptr<proto::trace::Event> getEventMessagePrototype() {
        return std::make_shared<proto::trace::Event>();
    }

protected:
    virtual bool compareEvents(const proto::trace::Event *a,
                               const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }

protected:
    TraceShRef m_trace;
};
}  // namespace octf

#endif /* SOURCE_OCTF_INTERFACE_INTERNAL_IOTRACEPARSER_H */
