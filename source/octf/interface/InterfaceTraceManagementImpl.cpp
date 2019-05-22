/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/interface/InterfaceTraceManagementImpl.h>

#include <dirent.h>
#include <string>
#include <octf/interface/TraceManager.h>
#include <octf/proto/InterfaceTraceManagement.pb.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

constexpr char InterfaceTraceManagementImpl::ASTERISK;

InterfaceTraceManagementImpl::InterfaceTraceManagementImpl(
        std::string tracePrefix) {
    m_tracePrefix = tracePrefix;
};

InterfaceTraceManagementImpl::~InterfaceTraceManagementImpl(){};

void InterfaceTraceManagementImpl::listTraces(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::TracePathPrefix *request,
        ::octf::proto::TraceList *response,
        ::google::protobuf::Closure *done) {
    (void) controller;
    const std::string &traceRootDir = getFrameworkConfiguration().getTraceDir();
    std::string cliPrefix = request->prefix();
    bool matchMultipleTraces = false;

    if (cliPrefix.length() == 0) {
        // Empty prefix - match all traces
        matchMultipleTraces = true;
    } else if (cliPrefix.back() == ASTERISK) {
        // Remember user used an asterisk and remove it from prefix
        matchMultipleTraces = true;
        cliPrefix.pop_back();
    }

    // Get a list of directories in root trace directory
    std::list<std::string> traceDirs;
    fsutils::readDirectoryContentsRecursive(traceRootDir, traceDirs,
                                            fsutils::FileType::Directory);

    // Check for trace summaries in these directories
    proto::TraceSummary summary;
    for (const auto &dir : traceDirs) {
        ProtobufReaderWriter rw(traceRootDir + "/" + dir + "/" +
                                SUMMARY_FILE_NAME);

        // Succesfully read summary - valid trace directory, now check if
        // TracePath argument is in the beginning of the directory path
        if (rw.read(summary)) {
            // Check if summary is valid
            if (!isValidSummary(summary)) {
                log::cerr << "Invalid trace summary found in " + dir
                          << std::endl;
                continue;
            }

            // Check if this trace path has this node's prefix
            // and if it matches cli prefix
            if (isMatchingPrefix(dir, cliPrefix, matchMultipleTraces)) {
                auto trace = response->add_trace();
                trace->set_tracepath(dir);
                trace->set_state(summary.state());
            }
        }
    }
    done->Run();
}

void InterfaceTraceManagementImpl::removeTraces(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::TracePathPrefix *request,
        ::octf::proto::Void *response,
        ::google::protobuf::Closure *done) {
    (void) response;
    std::list<std::string> dirsToRemove;
    const std::string &traceRootDir = getFrameworkConfiguration().getTraceDir();
    std::string cliPrefix = request->prefix();
    proto::TraceSummary summary;
    bool matchMultipleTraces = false;

    if (cliPrefix == "") {
        controller->SetFailed("No trace files specified.");
        done->Run();
        return;
    }

    if (cliPrefix.back() == ASTERISK) {
        // Remember user used an asterisk and remove it from prefix
        matchMultipleTraces = true;
        cliPrefix.pop_back();
    }

    // Get a list of directories
    std::list<std::string> traceDirs;
    fsutils::readDirectoryContentsRecursive(traceRootDir, traceDirs,
                                            fsutils::FileType::Directory);

    std::string notRemovedPaths = "";
    std::string failMessage = "";
    for (const auto &dir : traceDirs) {
        // Check if this trace path has this node's prefix
        // and if it matches cliPrefix
        if (isMatchingPrefix(dir, cliPrefix, matchMultipleTraces)) {
            std::string summaryPath =
                    traceRootDir + "/" + dir + "/" + SUMMARY_FILE_NAME;
            ProtobufReaderWriter rw(summaryPath);
            // Check if a valid summary exists in directory
            if (rw.read(summary)) {
                // Remove only completed or traces with errors
                if (summary.state() == proto::TraceState::COMPLETE ||
                    summary.state() == proto::TraceState::ERROR) {
                    dirsToRemove.push_back(traceRootDir + "/" + dir);
                } else {
                    notRemovedPaths += dir + "\n";
                }
            }
        }
    }

    // There were some traces which were not removed
    if (notRemovedPaths != "") {
        failMessage +=
                "Skipping following traces which may still be running:\n";
        failMessage += notRemovedPaths;
    }

    // Remove directories matching prefixes and having summary file
    for (const auto &dir : dirsToRemove) {
        if (!fsutils::removeFile(dir)) {
            failMessage += "Could not remove trace: " + dir;
            break;
        }
    }

    if (failMessage != "") {
        controller->SetFailed(failMessage);
    }
    done->Run();
}
void InterfaceTraceManagementImpl::getTraceSummary(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::TracePath *request,
        ::octf::proto::TraceSummary *response,
        ::google::protobuf::Closure *done) {
    // Get directory with traces
    std::string traceRootDir = getFrameworkConfiguration().getTraceDir();

    // Check for existence of summary
    std::string summaryPath =
            traceRootDir + "/" + request->tracepath() + "/" + SUMMARY_FILE_NAME;
    ProtobufReaderWriter rw(summaryPath);

    // Try to read it
    if (!rw.read(*response)) {
        // Could not read summary
        controller->SetFailed("Could not find specified trace summary.");
    }

    done->Run();
}

bool InterfaceTraceManagementImpl::isValidSummary(
        const proto::TraceSummary &summary) {
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

bool InterfaceTraceManagementImpl::isMatchingPrefix(std::string traceDir,
                                                    std::string prefix,
                                                    bool matchMultiple) {
    // If trace directory name does not match this node's name - return false
    if (traceDir.compare(0, m_tracePrefix.size(), m_tracePrefix) != 0) {
        return false;
    }

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
