/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/communication/internal/RpcProxy.h>

#include <chrono>
#include <octf/communication/internal/ConnectionContext.h>

namespace octf {

RpcProxy::RpcProxy()
        : m_lock()
        , m_connCntx(nullptr)
        , m_connCntxAvailable() {}

RpcProxy::~RpcProxy() {}

void RpcProxy::setConnectionContext(const ConnectionContextShRef &connCntx) {
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_connCntx = connCntx;
    }

    m_connCntxAvailable.notify_all();
}

void RpcProxy::clearConnectionContext() {
    std::lock_guard<std::mutex> lock(m_lock);
    m_connCntx.reset();
}

void RpcProxy::sendMethod(MethodClientUniqueRef method) {
    ConnectionContextShRef context;

    {
        // Grab connection context
        std::lock_guard<std::mutex> lock(m_lock);
        context = m_connCntx;
    }

    if (!context) {
        waitForConnectionContext();

        // Grab connection context
        std::lock_guard<std::mutex> lock(m_lock);
        context = m_connCntx;
    }

    if (context) {
        // Forward method into connection context
        context->sendMethod(std::move(method));
    } else {
        // No connection context, so end method with failure
        method->fail("No connection");
    }
}

void RpcProxy::cancelClientMethod(google::protobuf::RpcController *controller) {
    ConnectionContextShRef context;
    {
        // Grab connection context
        std::lock_guard<std::mutex> lock(m_lock);
        context = m_connCntx;
    }

    if (context) {
        // Forward method cancellation into connection context
        context->cancelClientMethod(controller);
    }
}

void RpcProxy::waitForConnectionContext() {
    std::unique_lock<std::mutex> lock(m_lock);

    m_connCntxAvailable.wait_for(lock, std::chrono::milliseconds(500),
                                 [this] { return nullptr != m_connCntx; });
}

}  // namespace octf
