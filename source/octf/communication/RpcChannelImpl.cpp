/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/communication/RpcChannelImpl.h>

#include <octf/communication/internal/ConnectionContext.h>
#include <octf/communication/internal/MethodClient.h>
#include <octf/communication/internal/RpcProxy.h>
#include <octf/utils/Log.h>

namespace octf {

RpcChannelImpl::RpcChannelImpl(const NodeShRef &node,
                               const RpcProxyShRef &rpcProxy)
        : m_node(node)
        , m_rpcProxy(rpcProxy) {}

RpcChannelImpl::~RpcChannelImpl() {}

void RpcChannelImpl::CallMethod(
        const google::protobuf::MethodDescriptor *methodDesc,
        google::protobuf::RpcController *controller,
        const google::protobuf::Message *request,
        google::protobuf::Message *response,
        google::protobuf::Closure *closure) {
    if (nullptr == methodDesc) {
        // Incorrect usage
        log::cerr << "CallMethod error.\n";
        controller->SetFailed("No method specified for call.");
        closure->Run();
        return;
    } else {
        // Perform actual method call
        genericCallMethod(InterfaceId(methodDesc->service()),
                          methodDesc->index(), controller, request, response,
                          closure);
    }
}

void RpcChannelImpl::genericCallMethod(
        const InterfaceId &interface,
        const int methodId,
        google::protobuf::RpcController *controller,
        const google::protobuf::Message *request,
        google::protobuf::Message *response,
        google::protobuf::Closure *closure) {
    if (!m_rpcProxy) {
        controller->SetFailed("No active RPC proxy");
        closure->Run();
    } else {
        // Create Method
        MethodClientUniqueRef method = MethodClientUniqueRef(
                new MethodClient(m_node, interface, methodId, controller,
                                 request, response, closure));

        m_rpcProxy->sendMethod(std::move(method));
    }
}

}  // namespace octf
