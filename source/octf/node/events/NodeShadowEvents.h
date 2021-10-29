/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_NODESHADOWEVENTS_H
#define SOURCE_OCTF_NODE_EVENTS_NODESHADOWEVENTS_H

#include "NodeEventGeneric.h"

namespace octf {

/**
 * @brief On node shadow attach to the node
 *
 * This event represents attaching a node shadow (client side) to the node
 * (server side). Attaching means establishment of communication channel between
 * client and server side.
 */
class NodeShadowEventOnAttach : public NodeEventGeneric {
public:
    NodeShadowEventOnAttach()
            : NodeEventGeneric("Node Shadow On Attach") {}
    virtual ~NodeShadowEventOnAttach() = default;
};

/**
 * @brief On node shadow attach to the node
 *
 * This event represents detaching a node shadow (client side) from the node
 * (server side). Detaching means communication channel has been terminated
 * between client and server side.
 */
class NodeShadowEventOnDetach : public NodeEventGeneric {
public:
    NodeShadowEventOnDetach()
            : NodeEventGeneric("Node Shadow On Detach") {}
    virtual ~NodeShadowEventOnDetach() = default;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_NODESHADOWEVENTS_H
