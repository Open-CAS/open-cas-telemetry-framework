/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/node/events/INodeEventHandler.h>

#include <octf/node/internal/NodeShadowEventDispatcher.h>

namespace octf {

NodeShadowEventDispatcher::NodeShadowEventDispatcher()
        : INodeEventDispatcher()
        , m_rmutex()
        , m_handlers()
        , m_runThread(true)
        , m_eventsQueue()
        , m_eventsInQueueSignal()
        , m_dispatchThread() {
    m_dispatchThread = std::thread([this]() {
        while (m_runThread) {
            m_eventsInQueueSignal.wait();

            processEvent();
        }
    });
}

NodeShadowEventDispatcher::~NodeShadowEventDispatcher() {
    clear();
}

void NodeShadowEventDispatcher::dispatchEvent(NodeEventShRef event) {
    std::lock_guard<std::recursive_mutex> lock(m_rmutex);
    m_eventsQueue.push_back(event);
    m_eventsInQueueSignal.post();
}

void NodeShadowEventDispatcher::registerEventHandler(
        NodeEventHandlerShRef handler) {
    std::lock_guard<std::recursive_mutex> lock(m_rmutex);

    m_handlers.insert(handler);
}

void NodeShadowEventDispatcher::unregisterEventHandler(
        NodeEventHandlerShRef handler) {
    std::lock_guard<std::recursive_mutex> lock(m_rmutex);

    m_handlers.erase(handler);
}

void NodeShadowEventDispatcher::clear() {
    {
        std::lock_guard<std::recursive_mutex> lock(m_rmutex);
        m_handlers.clear();

        m_runThread = false;
        m_eventsInQueueSignal.post();
    }

    if (m_dispatchThread.joinable()) {
        m_dispatchThread.join();
    }
}

void NodeShadowEventDispatcher::processEvent() {
    std::unordered_set<NodeEventHandlerShRef> handlers;
    NodeEventShRef event(nullptr);

    {
        std::lock_guard<std::recursive_mutex> lock(m_rmutex);

        if (!m_eventsQueue.size()) {
            // No events in the queue
            return;
        }

        // Make a copy of handlers and iterate on copied list, it will prevent
        // against list corruption when calling onEvent and adding new handler
        // inside
        handlers = m_handlers;

        // Fetch event from queue
        event = m_eventsQueue.front();
        m_eventsQueue.pop_front();
    }

    for (const auto &hndlr : handlers) {
        if (hndlr->getEventFilter().isEnabled(*event)) {
            hndlr->onEvent(event);
        }
    }
}

}  // namespace octf
