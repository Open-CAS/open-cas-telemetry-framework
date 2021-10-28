/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_ITRACEREPOSITORY_H
#define SOURCE_OCTF_TRACE_ITRACEREPOSITORY_H

#include <list>
#include <string>
#include <octf/trace/ITrace.h>

namespace octf {

class ITraceRepository {
public:
    ITraceRepository() = default;
    virtual ~ITraceRepository() = default;

    /**
     * @brief Gets existing trace
     *
     * @param tracePath Path to trace
     *
     * @return Shared reference of trace object
     */
    virtual TraceShRef getTrace(const std::string &tracePath) = 0;

    /**
     * @brief Gets existing traces by trace Prefix
     *
     * @param tracePath Path to trace
     *
     * @return Shared reference of trace object
     */
    virtual void getTraceList(const std::string &tracePrefix,
                              std::list<TraceShRef> &traceList) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_ITRACEREPOSITORY_H
