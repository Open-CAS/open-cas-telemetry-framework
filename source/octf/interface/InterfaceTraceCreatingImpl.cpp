/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceTraceCreatingImpl.h>

#include <string>

#include <octf/interface/ITraceSerializer.h>
#include <octf/interface/TraceManager.h>
#include <octf/utils/Exception.h>

namespace octf {

InterfaceTraceCreatingImpl::InterfaceTraceCreatingImpl(
        const NodePath &ownerNodePath,
        ITraceExecutor *traceExecutor)
        : m_traceManager(std::unique_ptr<TraceManager>(
                  new TraceManager(ownerNodePath, traceExecutor)))
        , m_ownerNodePath(ownerNodePath) {}

InterfaceTraceCreatingImpl::~InterfaceTraceCreatingImpl() = default;

void InterfaceTraceCreatingImpl::StartTracing(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::StartTraceRequest *request,
        ::octf::proto::Void *response,
        ::google::protobuf::Closure *done) {
    (void) response;
    try {
        auto maxDuration = request->maxduration();
        auto maxFileSize = request->maxsize();
        auto circBufferSize = request->circbuffersize();
        const auto &descriptor = request->descriptor();
        bool validData = true;
        if (!checkIntegerParameters(maxDuration, "maxduration", descriptor)) {
            validData = false;
            controller->SetFailed("Invalid maximum trace duration");
        }
        if (!checkIntegerParameters(circBufferSize, "circbuffersize",
                                    descriptor)) {
            validData = false;
            controller->SetFailed("Invalid circular buffer size");
        }
        if (!checkIntegerParameters(maxFileSize, "maxsize", descriptor)) {
            validData = false;
            controller->SetFailed("Invalid maximum trace file size");
        }

        if (validData) {
            // TODO (kozlowsk) return error code and status to user here
            m_traceManager->startJobs(maxDuration, maxFileSize, circBufferSize,
                                      request->label(),
                                      SerializerType::FileSerializer);
        }

    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    done->Run();
}

void InterfaceTraceCreatingImpl::StopTracing(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::TraceSummary *response,
        ::google::protobuf::Closure *done) {
    (void) request;
    try {
        m_traceManager->stopJobs();
        fillTraceSummary(response);
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }
    done->Run();
}

void InterfaceTraceCreatingImpl::GetTraceSummary(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::TraceSummary *status,
        ::google::protobuf::Closure *done) {
    (void) request;
    try {
        fillTraceSummary(status);
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }
    done->Run();
}

int InterfaceTraceCreatingImpl::pushTrace(uint32_t traceQueueId,
                                          const void *trace,
                                          const uint32_t traceSize) {
    return m_traceManager->pushTrace(traceQueueId, trace, traceSize);
}

bool InterfaceTraceCreatingImpl::checkIntegerParameters(
        const uint32_t value,
        const std::string &fieldName,
        const ::google::protobuf::Descriptor *bufferDescriptor) {
    const auto field = bufferDescriptor->FindFieldByLowercaseName(fieldName);
    const auto &valueInfo =
            field->options().GetExtension(proto::opts_param).cli_num();

    return (valueInfo.min() <= value) && (value <= valueInfo.max());
}

void InterfaceTraceCreatingImpl::fillTraceSummary(
        proto::TraceSummary *summary) {
    TracingState state = m_traceManager->getState();
    m_traceManager->fillTraceSummary(summary, state);
}

}  // namespace octf
