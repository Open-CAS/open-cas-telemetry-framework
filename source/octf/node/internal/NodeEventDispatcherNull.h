/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_INTERNAL_NODEEVENTDISPATCHERNULL_H
#define SOURCE_OCTF_NODE_INTERNAL_NODEEVENTDISPATCHERNULL_H

#include <octf/node/events/INodeEventDispatcher.h>

namespace octf {

/**
 * @brief Empty ("NULL") implementation of event handler that ignores all events
 */
class NodeEventDispatcherNull : public INodeEventDispatcher {
public:
    NodeEventDispatcherNull() = default;
    virtual ~NodeEventDispatcherNull() = default;

    void dispatchEvent(NodeEventShRef __attribute__((__unused__))
                       event) override{};

    void registerEventHandler(NodeEventHandlerShRef __attribute__((__unused__))
                              eventHandler) override{};

    void unregisterEventHandler(NodeEventHandlerShRef
                                __attribute__((__unused__)) handler) override{};
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_INTERNAL_NODEEVENTDISPATCHERNULL_H
