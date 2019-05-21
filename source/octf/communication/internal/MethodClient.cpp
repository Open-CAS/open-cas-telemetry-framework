/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/communication/internal/MethodClient.h>

#include <octf/proto/packets.pb.h>
#include <octf/socket/ConnectionTransceiver.h>
#include <octf/utils/Log.h>

namespace octf {

MethodClient::MethodClient(const NodeWeakRef &node,
                           const InterfaceId &interface,
                           int methodId,
                           google::protobuf::RpcController *controller,
                           const google::protobuf::Message *request,
                           google::protobuf::Message *response,
                           google::protobuf::Closure *closure)
        : m_interface(interface)
        , m_mId(methodId)
        , m_node(node)
        , m_sid()
        , m_methodDescriptor(nullptr)
        , m_rpcController(controller)
        , m_request(request)
        , m_response(response)
        , m_closure(closure) {}

MethodClient::~MethodClient() {}

bool MethodClient::preparePacket(proto::Packet &packet) {
    NodeShRef node = m_node.lock();
    if (!node) {
        return false;
    }

    auto *method = packet.mutable_methodrequest();

    const NodePath &path = node->getNodePath();
    for (const auto &iter : path) {
        proto::NodeId *id = method->mutable_path()->add_node();
        id->set_id(iter.getId());
    }

    method->mutable_interface()->set_name(m_interface.getName());
    method->mutable_interface()->set_version(m_interface.getVersion());

    // Set method id
    method->set_mid(m_mId);
    // Set sequence id
    method->set_sid(m_sid);

    // Serialize request into payload;
    if (!m_request->SerializeToString(method->mutable_request())) {
        return false;
    }

    return true;
}

void MethodClient::fail(const std::string &reason) {
    // Check if node reference is valid
    auto node = m_node.lock();
    if (!node) {
        return;
    }

    m_rpcController->SetFailed(reason);
    m_closure->Run();
}

void MethodClient::complete(const std::string &response) {
    // Check if node reference is valid
    auto node = m_node.lock();
    if (!node) {
        return;
    }

    if (m_response->ParseFromString(response)) {
        m_closure->Run();
    } else {
        fail("ERROR, Cannot parse response");
    }
}

bool MethodClient::compareController(
        google::protobuf::RpcController *controller) {
    if (m_rpcController == controller) {
        return true;
    } else {
        return false;
    }
}

}  // namespace octf
