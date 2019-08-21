/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/parser/TraceEventHandlerWorkset.h>

namespace octf {

CasTraceEventHandlerWorkset::CasTraceEventHandlerWorkset(
        const std::string &tracePath)
        : TraceEventHandler(tracePath)
        , m_calc() {}

CasTraceEventHandlerWorkset::~CasTraceEventHandlerWorkset() {}

void CasTraceEventHandlerWorkset::handleEvent(EventShRef traceEvent) {
    if (traceEvent->has_io()) {
        const auto &io = traceEvent->io();
        m_calc.insertRange(io.lba(), io.len());
    }
}

uint64_t CasTraceEventHandlerWorkset::getWorkset() {
    return m_calc.getWorkset();
}

}  // namespace octf
