/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/node/NodeServer.h>

#include <memory>

#include <octf/communication/CommunicationManagerServer.h>
#include <octf/node/internal/MethodHandler.h>
#include <octf/socket/SocketConfig.h>
#include <octf/utils/FrameworkConfiguration.h>

namespace octf {

NodeServer::NodeServer(const NodeId &id)
        : NodeBase(id)
        , m_commMngr(new CommunicationManagerServer(
                  std::unique_ptr<MethodHandler>(new MethodHandler(this)))) {}

NodeServer::~NodeServer() {}

bool NodeServer::initCommon() {
    // Call init of parent class
    if (!NodeBase::initCommon()) {
        return false;
    }

    m_commMngr->startSocket(SocketConfig(
            getFrameworkConfiguration().getUnixSocketFilePath(getNodeId()),
            SocketImplementation::Unix, SocketType::Server));

    return true;
}

void NodeServer::deinitCommon() {
    m_commMngr->shutdown();

    // Call deinit of parent class
    NodeBase::deinitCommon();
}

bool NodeServer::initCustom() {
    return true;
}

void NodeServer::deinitCustom() {}

}  // namespace octf
