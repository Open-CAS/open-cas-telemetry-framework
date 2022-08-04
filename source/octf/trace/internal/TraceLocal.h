/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H

#include <memory>
#include <octf/trace/internal/TraceBase.h>
#include <octf/trace/internal/TraceCacheLocal.h>

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

    ITraceCache &getCache() override;

    void getExtensionList(std::list<std::string> &extList) override;

    TraceExtensionShRef getExtension(const std::string &name) override;

private:
    proto::TraceSummary readSummary(const std::string &tracePath) const;
    bool isValidSummary(const proto::TraceSummary &summary) const;

private:
    const std::string m_dir;
    const std::string m_path;
    const proto::TraceSummary m_summary;
    std::unique_ptr<TraceCacheLocal> m_cache;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACELOCAL_H
