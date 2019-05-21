/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_INTERNAL_NODESHADOWEVENTDISPATCHER_H
#define SOURCE_OCTF_NODE_INTERNAL_NODESHADOWEVENTDISPATCHER_H

#include <atomic>
#include <list>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <octf/node/events/INodeEventDispatcher.h>
#include <octf/node/events/NodeEventFilter.h>
#include <octf/utils/Semaphore.h>

namespace octf {

/**
 * @brief Node shadow event handler implementation
 */
class NodeShadowEventDispatcher : public INodeEventDispatcher {
public:
    NodeShadowEventDispatcher();
    virtual ~NodeShadowEventDispatcher();

    void dispatchEvent(NodeEventShRef event) override;

    void registerEventHandler(NodeEventHandlerShRef eventHandler) override;

    void unregisterEventHandler(NodeEventHandlerShRef handler) override;

private:
    void clear();

    void processEvent();

private:
    /**
     * @brief Recursive mutex for locking list of handlers
     *
     * We use recursive because during handling an event, a handler might
     * register new one, so then it will take mutex recursively.
     */
    std::recursive_mutex m_rmutex;

    /**
     * Set of registered handlers
     */
    std::unordered_set<NodeEventHandlerShRef> m_handlers;

    /**
     * Boolean variable for indicating if dispatch thread shall be run
     */
    std::atomic_bool m_runThread;

    /**
     * Queue of events to be dispatched
     */
    std::list<NodeEventShRef> m_eventsQueue;

    /**
     * Semaphore for signaling if events is in the queue
     */
    Semaphore m_eventsInQueueSignal;

    /**
     * Thread which pops events from queue and dispatches them to handlers
     */
    std::thread m_dispatchThread;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_INTERNAL_NODESHADOWEVENTDISPATCHER_H
