/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/plugin/NodePlugin.h>

#include <chrono>
#include <thread>
#include <octf/plugin/internal/ServiceShadow.h>

namespace octf {

NodePlugin::NodePlugin(const NodeId &id)
        : NodeServer(id)
        , m_nodeService(std::make_shared<ServiceShadow>()) {}

NodePlugin::~NodePlugin() {}

bool NodePlugin::initCommon() {
    // Call common init of parent class
    if (!NodeServer::initCommon()) {
        return false;
    }

    m_nodeService->enablePluginAutoRegistration(getNodeId());

    if (!m_nodeService->init()) {
        return false;
    }

    return true;
}

void NodePlugin::deinitCommon() {
    m_nodeService->disablePluginAutoRegistration(getNodeId());
    m_nodeService->deinit();
    NodeServer::deinitCommon();
}

}  // namespace octf
