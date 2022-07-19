/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_ITRACE_H
#define SOURCE_OCTF_TRACE_ITRACE_H

#include <memory>
#include <string>
#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/ITraceCache.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

class ITrace : public NonCopyable {
public:
    ITrace() = default;
    virtual ~ITrace() = default;

    virtual const std::string &getPath() const = 0;

    virtual const proto::TraceSummary &getSummary() const = 0;

    virtual void remove(bool force = true) = 0;

    virtual bool isTracingEnd() const = 0;

    /**
     * @brief Gets the trace cache
     *
     * To avoid long trace processing some results can be persisted to the cache
     *
     * @return The trace cache
     */
    virtual ITraceCache &getCache() = 0;
};

/**
 * @typedef Shared reference to a trace object
 */
typedef std::shared_ptr<ITrace> TraceShRef;

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_ITRACE_H
