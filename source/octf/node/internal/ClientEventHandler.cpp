/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/node/NodeShadow.h>
#include <octf/node/events/NodeShadowEvents.h>
#include <octf/node/internal/ClientEventHandler.h>
#include <octf/utils/Log.h>

namespace octf {

ClientEventHandler::ClientEventHandler(INode *owner)
        : IClientEventHandler()
        , m_owner(owner) {}

void ClientEventHandler::onAttach() {
    auto &dispatcher = m_owner->getEventDispatcher();
    auto event = std::make_shared<NodeShadowEventOnAttach>();

    dispatcher.dispatchEvent(event);
}

void ClientEventHandler::onDetach() {
    auto &dispatcher = m_owner->getEventDispatcher();
    auto event = std::make_shared<NodeShadowEventOnDetach>();

    dispatcher.dispatchEvent(event);
}

}  // namespace octf
