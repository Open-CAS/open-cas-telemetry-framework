/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/interface/InterfaceTraceManagementImpl.h>

#include <dirent.h>
#include <string>
#include <octf/communication/RpcOutputStream.h>
#include <octf/interface/TraceManager.h>
#include <octf/proto/InterfaceTraceManagement.pb.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

InterfaceTraceManagementImpl::InterfaceTraceManagementImpl(
        std::string tracePrefix) {
    m_tracePrefix = tracePrefix;
};

InterfaceTraceManagementImpl::~InterfaceTraceManagementImpl(){};

void InterfaceTraceManagementImpl::listTraces(
        ::google::protobuf::RpcController *,
        const ::octf::proto::TracePathPrefix *request,
        ::octf::proto::TraceList *response,
        ::google::protobuf::Closure *done) {
    const auto &prefix = request->prefix();

    // If trace directory name does not match this node's name
    if (prefix.compare(0, m_tracePrefix.size(), m_tracePrefix) != 0) {
        done->Run();
        return;
    }

    std::list<TraceShRef> traceList;
    TraceLibrary::get().getTraceList(prefix, traceList);

    for (auto const &trace : traceList) {
        auto itme = response->add_trace();
        itme->set_tracepath(trace->getSummary().tracepath());
        itme->set_state(trace->getSummary().state());
        *itme->mutable_tags() = trace->getSummary().tags();
    }

    done->Run();
}

void InterfaceTraceManagementImpl::removeTraces(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::RemoveTracesRequest *request,
        ::octf::proto::TraceList *response,
        ::google::protobuf::Closure *done) {
    const auto &prefix = request->prefix();
    RpcOutputStream cerr(log::Severity::Error, controller);

    // If trace directory name does not match this node's name
    if (prefix.compare(0, m_tracePrefix.size(), m_tracePrefix) != 0) {
        controller->SetFailed("No traces removed.");
        done->Run();
        return;
    }

    std::list<TraceShRef> tracesToRemove;
    TraceLibrary::get().getTraceList(prefix, tracesToRemove);

    if (!request->force()) {
        // We are allowed to remove only done traces
        auto iter = tracesToRemove.begin();
        for (; iter != tracesToRemove.end();) {
            if ((*iter)->isTracingEnd()) {
                iter++;
            } else {
                iter = tracesToRemove.erase(iter);
            }
        }
    }

    for (auto &trace : tracesToRemove) {
        try {
            trace->remove(request->force());

            // Add removed traces to response
            auto removedTrace = response->add_trace();
            removedTrace->set_tracepath(trace->getSummary().tracepath());
            removedTrace->set_state(trace->getSummary().state());
            *removedTrace->mutable_tags() = trace->getSummary().tags();
        } catch (Exception &e) {
            cerr << e.getMessage() << std::endl;
        }
    }

    // Set fail only when no traces were removed.
    if (response->trace_size() == 0) {
        controller->SetFailed("No traces removed.");
    }

    done->Run();
}
void InterfaceTraceManagementImpl::getTraceSummary(
        ::google::protobuf::RpcController *,
        const ::octf::proto::TracePath *request,
        ::octf::proto::TraceSummary *response,
        ::google::protobuf::Closure *done) {
    auto trace = TraceLibrary::get().getTrace(request->tracepath());
    response->CopyFrom(trace->getSummary());
    done->Run();
}

void InterfaceTraceManagementImpl::clearTraceCache(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::TracePathPrefix *request,
        ::octf::proto::TraceList *response,
        ::google::protobuf::Closure *done) {
    try {
        const auto &prefix = request->prefix();

        // If trace directory name does not match this node's name
        if (prefix.compare(0, m_tracePrefix.size(), m_tracePrefix) != 0) {
            done->Run();
            return;
        }

        std::list<TraceShRef> traceList;
        TraceLibrary::get().getTraceList(prefix, traceList);

        for (auto const &trace : traceList) {
            auto &cache = trace->getCache();
            cache.clear();
            auto item = response->add_trace();
            item->set_tracepath(trace->getSummary().tracepath());
            item->set_state(trace->getSummary().state());
            *item->mutable_tags() = trace->getSummary().tags();
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
