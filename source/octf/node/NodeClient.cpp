/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/node/NodeClient.h>

#include <octf/communication/CommunicationManagerClient.h>
#include <octf/node/internal/ClientEventHandler.h>
#include <octf/socket/SocketConfig.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>

namespace octf {

NodeClient::NodeClient(const NodeId &id)
        : NodeShadow(id)
        , m_commMngr(new CommunicationManagerClient(
                  std::unique_ptr<IClientEventHandler>(
                          new ClientEventHandler(this)))) {}

NodeClient::~NodeClient() {}

const RpcProxyShRef &NodeClient::getRpcProxy() {
    return m_commMngr->getRpcProxy();
}

bool NodeClient::initCommon() {
    // Call init of parent class
    if (!NodeShadow::initCommon()) {
        return false;
    }

    if (!getRpcChannel()) {
        // No RPC channel, first set it and then call initialization
        throw Exception("RPC channel not set.");
    }

    m_commMngr->startSocket(SocketConfig(
            getFrameworkConfiguration().getUnixSocketFilePath(getNodeId()),
            SocketImplementation::Unix, SocketType::Client));

    return true;
}

void NodeClient::deinitCommon() {
    m_commMngr->shutdown();
}

bool NodeClient::initCustom() {
    return true;
}

void NodeClient::deinitCustom() {}

}  // namespace octf
