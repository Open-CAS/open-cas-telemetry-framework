/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERFUNCTION_H
#define SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERFUNCTION_H

#include <functional>
#include <octf/node/events/NodeEventHandlerGeneric.h>

#include <octf/node/events/NodeShadowEvents.h>

namespace octf {

/**
 * @brief Base class of node event handler which calls function on event
 * occurrence
 *
 * Example code:
 * @code
 * void setupEventHandler(NodePluginShadow *plugin) {
 *     // Define your function for handling events. In this case two events
 *     // to be handled, one for attaching, second for detaching
 *     auto fn = [](NodeEventShRef event) {
 *         if (event->isInstanceOf<NodeShadowEventOnAttach>()) {
 *             // Do your action on attach
 *         } else if (event->isInstanceOf<NodeShadowEventOnDetach>()) {
 *             // Do your action on detach
 *         }
 *     };
 *
 *     // Create handler
 *     auto hndlr = std::make_shared<NodeEventHandlerFunction>(fn);
 *
 *     // Enables event types to be handled by this handler
 *     hndlr->getEventFilter().enable<NodeShadowEventOnDetach>();
 *     hndlr->getEventFilter().enable<NodeShadowEventOnAttach>();
 *
 *     // Get node dispatcher and register function handler
 *     plugin->getEventDispatcher().registerEventHandler(hndlr);
 * }
 * @endcode
 */
class NodeEventHandlerFunction : public NodeEventHandlerGeneric {
public:
    /**
     * @typedef Function handler which is called when event occurs
     */
    typedef std::function<void(NodeEventShRef)> FunctionHandler;

    /**
     * @param fn Function to be called when event occurs
     */
    NodeEventHandlerFunction(FunctionHandler fn)
            : NodeEventHandlerGeneric()
            , m_fn(fn) {}
    virtual ~NodeEventHandlerFunction() = default;

    void onEvent(NodeEventShRef event) {
        m_fn(event);
    }

private:
    FunctionHandler m_fn;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERFUNCTION_H
