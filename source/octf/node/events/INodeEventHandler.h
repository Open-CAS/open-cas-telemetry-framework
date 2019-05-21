/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_INODEEVENTHANDLER_H
#define SOURCE_OCTF_NODE_EVENTS_INODEEVENTHANDLER_H

#include <memory>
#include <octf/node/events/INodeEvent.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

class NodeEventFilter;

/**
 * @interface INodeEventHanlder
 *
 * @brief Interface responsible for handling node events which are enabled
 * in event filter
 */
class INodeEventHandler : public NonCopyable {
public:
    INodeEventHandler() = default;
    virtual ~INodeEventHandler() = default;

    /**
     * @brief On node event
     *
     * @param event Event to be handled
     */
    virtual void onEvent(NodeEventShRef event) = 0;

    /**
     * @brief Gets events filter
     *
     * Node event filter indicates which events type can be handled by this
     * event handler
     *
     * @return Node events filter
     */
    virtual NodeEventFilter &getEventFilter() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_INODEEVENTHANDLER_H
