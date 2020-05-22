/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/internal/TraceRepositoryLocal.h>

#include <octf/trace/internal/TraceLocal.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

TraceShRef TraceRepositoryLocal::getTrace(const std::string &tracePath) {
    return std::make_shared<TraceLocal>(tracePath);
}

void TraceRepositoryLocal::getTraceList(const std::string &tracePrefix,
                                        std::list<TraceShRef> &traceList) {
    const std::string &traceRootDir = getFrameworkConfiguration().getTraceDir();
    bool matchMultipleTraces = false;
    constexpr char ASTERISK = '*';
    auto prefix = tracePrefix;

    if (prefix.length() == 0) {
        // Empty prefix - match all traces
        matchMultipleTraces = true;
    } else if (prefix.back() == ASTERISK) {
        // Remember user used an asterisk and remove it from prefix
        matchMultipleTraces = true;
        prefix.pop_back();
    }

    // Get a list of directories in root trace directory
    std::list<std::string> traceDirs;
    fsutils::readDirectoryContentsRecursive(traceRootDir, traceDirs,
                                            fsutils::FileType::Directory);

    // Check for trace summaries in these directories
    proto::TraceSummary summary;
    traceDirs.sort();

    for (const auto &dir : traceDirs) {
        try {
            auto trace = getTrace(dir);

            if (isMatchingPrefix(dir, prefix, matchMultipleTraces)) {
                traceList.emplace_back(trace);
            }
        } catch (Exception &) {
        }
    }
}

bool TraceRepositoryLocal::isMatchingPrefix(std::string traceDir,
                                            std::string prefix,
                                            bool matchMultiple) const {
    if (matchMultiple) {
        // We want to match multiple traces
        // - match if prefix is in the beginning of traceDir
        if (!traceDir.compare(0, prefix.size(), prefix)) {
            return true;
        }
    } else {
        // We are not matching multiple traces
        // - match only if prefix is the same as complete path
        if (traceDir.size() == prefix.length() &&
            !traceDir.compare(0, traceDir.length(), prefix)) {
            return true;
        }
    }
    return false;
}

}  // namespace octf
