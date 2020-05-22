/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/internal/TraceBase.h>

namespace octf {

bool TraceBase::isTracingEnd() const {
    const auto &summary = getSummary();
    if (summary.state() == proto::TraceState::COMPLETE ||
        summary.state() == proto::TraceState::ERROR) {
        return true;
    }

    return false;
}

}  // namespace octf
