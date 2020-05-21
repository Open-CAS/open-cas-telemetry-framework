/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACEBASE_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACEBASE_H

#include <octf/trace/ITrace.h>

namespace octf {

class TraceBase : public ITrace {
public:
    TraceBase() = default;
    virtual ~TraceBase() = default;

    virtual bool isTracingEnd() const override;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACEBASE_H
