/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_INODEEVENTDISPATCHER_H
#define SOURCE_OCTF_NODE_EVENTS_INODEEVENTDISPATCHER_H

#include <functional>
#include <memory>
#include <octf/node/events/INodeEvent.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @interface INodeEventDispatcher
 *
 * @brief Interface responsible for dispatching a node event to registered
 * handlers
 */
class INodeEventDispatcher : public NonCopyable {
public:
    INodeEventDispatcher() = default;
    virtual ~INodeEventDispatcher() = default;

    /**
     * @brief Dispatches event to registered handlers
     *
     * @param event Event to be dispatched to handlers
     */
    virtual void dispatchEvent(NodeEventShRef event) = 0;

    /**
     * @brief Registers node event handler to dispatcher
     *
     * @param handler Node event handler to be registered
     */
    virtual void registerEventHandler(NodeEventHandlerShRef handler) = 0;

    /**
     * @brief Unregisters node event handler from dispatcher
     *
     * @param handler Node event handler to be unregistered
     */
    virtual void unregisterEventHandler(NodeEventHandlerShRef handler) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_INODEEVENTDISPATCHER_H
