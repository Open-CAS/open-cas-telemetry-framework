/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/node/NodeShadow.h>

#include <google/protobuf/service.h>
#include <octf/proto/InterfaceIdentification.pb.h>

namespace octf {

NodeShadow::NodeShadow(const NodeId &id)
        : NodeGeneric(id)
        , m_rpcChannel(nullptr) {}

NodeShadow::~NodeShadow(){};

bool NodeShadow::addChild(NodeShRef child) {
    auto shadowChild = std::dynamic_pointer_cast<NodeShadow>(child);
    if (shadowChild) {
        // Setup RPC channel in child
        shadowChild->m_rpcChannel = std::make_shared<RpcChannelImpl>(
                child, m_rpcChannel->getRpcProxy());
    } else {
        // This is different implementation of node, so it will take care for
        // RPC channel on its own.
    }

    if (NodeGeneric::addChild(child)) {
        return true;
    } else {
        // Error while adding child, deinitialize it in order to cleanup
        child->deinit();
        return false;
    }
}

bool NodeShadow::initCommon() {
    // Call common init of parent class
    if (!NodeGeneric::initCommon()) {
        return false;
    }

    if (!createInterface<proto::InterfaceIdentification_Stub>()) {
        // Cannot add identification interface
        return false;
    }

    return true;
}

}  // namespace octf
