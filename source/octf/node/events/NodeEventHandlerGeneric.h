/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERGENERIC_H
#define SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERGENERIC_H

#include <octf/node/events/INodeEventHandler.h>
#include <octf/node/events/NodeEventFilter.h>
#include <octf/utils/Log.h>

namespace octf {

/**
 * @brief Base class for INodeEventHanlder implementation
 */
class NodeEventHandlerGeneric : public INodeEventHandler {
public:
    NodeEventHandlerGeneric()
            : INodeEventHandler()
            , m_filter() {}

    virtual ~NodeEventHandlerGeneric() = default;

    NodeEventFilter &getEventFilter() override {
        return m_filter;
    }

protected:
    /**
     * Node event filter
     */
    NodeEventFilter m_filter;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_NODEEVENTHANDLERGENERIC_H
