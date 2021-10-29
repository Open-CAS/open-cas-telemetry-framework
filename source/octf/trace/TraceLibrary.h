/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_TRACELIBRARY_H
#define SOURCE_OCTF_TRACE_TRACELIBRARY_H

#include <memory>
#include <string>
#include <octf/trace/ITrace.h>
#include <octf/trace/ITraceRepository.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * Utilities which shall be used to create given Socket implementation
 */
class TraceLibrary : private NonCopyable, public ITraceRepository {
public:
    virtual ~TraceLibrary() = default;
    static TraceLibrary &get();

    TraceShRef getTrace(const std::string &tracePath) override;

    void getTraceList(const std::string &tracePrefix,
                      std::list<TraceShRef> &traceList) override;

private:
    TraceLibrary() = default;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_TRACELIBRARY_H
