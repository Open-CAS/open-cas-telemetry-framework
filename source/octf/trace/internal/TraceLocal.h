/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H

#include <octf/trace/internal/TraceBase.h>

namespace octf {

class TraceLocal : public TraceBase {
public:
    TraceLocal(const std::string &tracePath);
    virtual ~TraceLocal() = default;

    const octf::proto::TraceSummary &getSummary() const override {
        return m_summary;
    }

    const std::string &getPath() const override {
        return m_path;
    }

    void remove(bool force) override;

private:
    proto::TraceSummary readSummary(const std::string &tracePath) const;
    bool isValidSummary(const proto::TraceSummary &summary) const;

private:
    const std::string m_dir;
    const std::string m_path;
    const proto::TraceSummary m_summary;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H
