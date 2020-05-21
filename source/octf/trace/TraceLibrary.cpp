/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/TraceLibrary.h>
#include <octf/trace/internal/TraceRepositoryLocal.h>

namespace octf {

TraceLibrary &TraceLibrary::get() {
    static TraceLibrary mngr;

    return mngr;
}

TraceShRef TraceLibrary::getTrace(const std::string &tracePath) {
    return TraceRepositoryLocal().getTrace(tracePath);
}

void octf::TraceLibrary::getTraceList(const std::string &tracePrefix,
                                      std::list<TraceShRef> &traceList) {
    TraceRepositoryLocal().getTraceList(tracePrefix, traceList);
}

}  // namespace octf
