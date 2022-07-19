/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/internal/TraceLocal.h>

#include <octf/interface/TraceManager.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

TraceLocal::TraceLocal(const std::string &tracePath)
        : TraceBase()
        , m_dir(getFrameworkConfiguration().getTraceDir())
        , m_path(tracePath)
        , m_summary(readSummary(tracePath))
        , m_cache(nullptr) {
    if (!isValidSummary(m_summary)) {
        throw Exception("Invalid trace summary for trace path: " + tracePath);
    }
}

proto::TraceSummary TraceLocal::readSummary(
        const std::string &tracePath) const {
    proto::TraceSummary summary;

    // Check for existence of summary
    std::string summaryPath = m_dir + "/" + tracePath + "/" + SUMMARY_FILE_NAME;
    ProtobufReaderWriter rw(summaryPath);

    // Try to read it
    if (!rw.read(summary)) {
        // Could not read summary
        throw Exception("Could not find summary for trace: " + tracePath);
    }

    summary.set_tracepath(tracePath);
    return summary;
}

void TraceLocal::remove(bool force) {
    if (!isTracingEnd()) {
        if (!force) {
            throw Exception("Invalid trace state, could not remove trace: " +
                            m_path);
        }
    }

    std::string absolutePath = m_dir + "/" + m_path;

    bool removed = fsutils::removeFile(absolutePath);
    if (!removed) {
        throw Exception("Could not remove trace: " + m_path);
    }
}

bool TraceLocal::isValidSummary(const proto::TraceSummary &summary) const {
    if (summary.sourcenode().node_size() == 0) {
        return false;
    }
    if (summary.queuecount() < 1) {
        return 0;
    }
    if (summary.traceduration() < 0) {
        return false;
    }
    if (summary.tracestartdatetime() == "") {
        return false;
    }

    return true;
}

ITraceCache &TraceLocal::getCache() {
    if (!m_cache) {
        m_cache.reset(new TraceCacheLocal(m_path));
    }

    return *m_cache;
}

}  // namespace octf
