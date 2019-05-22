/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_NODEEVENTGENERIC_H
#define SOURCE_OCTF_NODE_EVENTS_NODEEVENTGENERIC_H

#include <octf/node/events/INodeEvent.h>

namespace octf {

/**
 * @brief Base class for node events
 */
class NodeEventGeneric : public INodeEvent {
public:
    /**
     * @param eventName Event name
     */
    NodeEventGeneric(const std::string &eventName)
            : m_name(eventName) {}
    virtual ~NodeEventGeneric() = default;

    const std::string &getEventName() const override {
        return m_name;
    }

private:
    std::string m_name;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_NODEEVENTGENERIC_H
