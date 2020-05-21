/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACEREPOSITORYLOCAL_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACEREPOSITORYLOCAL_H

#include <string>
#include <octf/trace/ITraceRepository.h>

namespace octf {

class TraceRepositoryLocal : ITraceRepository {
public:
    TraceRepositoryLocal() = default;
    virtual ~TraceRepositoryLocal() = default;

    virtual octf::TraceShRef getTrace(const std::string &tracePath) override;

    void getTraceList(const std::string &tracePrefix,
                      std::list<TraceShRef> &traceList);

private:
    bool isMatchingPrefix(std::string traceDir,
                          std::string prefix,
                          bool matchMultiple) const;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACEREPOSITORYLOCAL_H
