/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_INODEEVENT_H
#define SOURCE_OCTF_NODE_EVENTS_INODEEVENT_H

#include <memory>
#include <string>
#include <typeinfo>
#include <octf/utils/NonCopyable.h>

namespace octf {

class INodeEvent;
typedef std::shared_ptr<INodeEvent> NodeEventShRef;

class INodeEventHandler;
typedef std::shared_ptr<INodeEventHandler> NodeEventHandlerShRef;

/**
 * @interface INodeEvent
 *
 * @brief Node event interface
 */
class INodeEvent : public NonCopyable {
public:
    INodeEvent() = default;
    virtual ~INodeEvent() = default;

    /**
     * @brief Gets event name
     *
     * @return Event name
     */
    virtual const std::string &getEventName() const = 0;

    /**
     * @brief Checks if specified event is instance of T
     *
     * @tparam T Event type to be matched specified event
     *
     * @param event Event to be checked
     *
     * @retval true Event is instance of T
     * @retval false Event is not instance of T
     */
    template <typename T>
    inline bool isInstanceOf() {
        if (typeid(T) == typeid(*this)) {
            return true;
        } else {
            return false;
        }
    }
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_INODEEVENT_H
