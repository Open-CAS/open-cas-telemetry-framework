/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/plugin/NodePluginShadow.h>

#include <octf/node/internal/NodeShadowEventDispatcher.h>
#include <octf/plugin/internal/NodePluginShadowWrapper.h>

namespace octf {

NodePluginShadow::NodePluginShadow(const NodeId &id)
        : NodeClient(id)
        , m_wrapper(nullptr)
        , m_dispatcher(nullptr)
        , m_state(PluginState::UNDEFINED)
        , m_description("")
        , m_error("") {}

NodePluginShadow::~NodePluginShadow() {}

const std::string &NodePluginShadow::getDescription() const {
    return m_description;
}

void NodePluginShadow::setDescription(const std::string &desc) {
    m_description = desc;
}

const std::string &NodePluginShadow::getError() const {
    return m_error;
}

void NodePluginShadow::setError(const std::string &error) {
    m_error = error;
}

bool NodePluginShadow::initCommon() {
    // RpcChannel requires shared pointer to a INode. Because NodePluginShadow
    // is not shared pointer usually, we use shared pointer wrapper. Wrapper
    // points on this node plugin shadow. It's safe because this object is root
    // and overlives all children objects.

    // Create wrapper for this node as shared pointer
    m_wrapper = std::make_shared<NodePluginShadowWrapper>(this);

    // Setup RPC channel for this node plugin shadow
    RpcChannelShRef rootRpcChannel =
            std::make_shared<RpcChannelImpl>(m_wrapper, getRpcProxy());

    // Set RPC channel
    setRpcChannel(rootRpcChannel);

    if (!NodeClient::initCommon()) {
        return false;
    }

    return true;
}

INodeEventDispatcher &NodePluginShadow::getEventDispatcher() {
    if (!m_dispatcher) {
        m_dispatcher.reset(new NodeShadowEventDispatcher());
    }

    return *m_dispatcher;
}

void NodePluginShadow::deinitCommon() {
    NodeClient::deinitCommon();

    // Release dispatcher with all references to handlers
    m_dispatcher.reset(nullptr);
}

bool NodePluginShadow::initCustom() {
    return true;
}

void NodePluginShadow::deinitCustom() {}

}  // namespace octf
