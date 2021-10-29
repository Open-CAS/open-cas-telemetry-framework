/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/communication/RpcControllerImpl.h>
#include <octf/communication/internal/RpcProxy.h>
#include <octf/plugin/NodePluginShadow.h>

namespace octf {

RpcControllerImpl::RpcControllerImpl()
        : m_err("")
        , m_cancelled(false)
        , m_rpcProxy(nullptr) {}

RpcControllerImpl::RpcControllerImpl(NodePluginShadow *node)
        : m_err("")
        , m_cancelled(false)
        , m_rpcProxy(nullptr) {
    const auto &channel = node->getRpcChannel();
    if (channel) {
        m_rpcProxy = channel->getRpcProxy();
    }
}

void RpcControllerImpl::Reset() {}

bool RpcControllerImpl::Failed() const {
    return !m_err.empty();
}

std::string RpcControllerImpl::ErrorText() const {
    return m_err;
}

void RpcControllerImpl::StartCancel() {
    if (m_rpcProxy) {
        m_rpcProxy->cancelClientMethod(this);
    }
    m_cancelled = true;
}

void RpcControllerImpl::SetFailed(const std::string &reason) {
    m_err = reason;
}

bool RpcControllerImpl::IsCanceled() const {
    return m_cancelled;
}

void RpcControllerImpl::NotifyOnCancel(google::protobuf::Closure *callback) {
    // method unused, required implementation because of abstract method.
    // use void cast to avoid compilation warning, it's intended implementation
    (void) callback;
}

log::OutputStream &octf::RpcControllerImpl::getOutputStream(
        log::Severity severity) {
    switch (severity) {
    case log::Severity::Information:
        return log::cout;

    case log::Severity::Verbose:
        return log::verbose;

    case log::Severity::Critical:
        return log::critical;

    case log::Severity::Error:
        return log::cerr;
        break;
    default:
        throw Exception("No RPC output stream for specific severity");
    }
}

}  // namespace octf
