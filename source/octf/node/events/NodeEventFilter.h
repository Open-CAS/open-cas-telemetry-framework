/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_EVENTS_NODEEVENTFILTER_H
#define SOURCE_OCTF_NODE_EVENTS_NODEEVENTFILTER_H

#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_set>
#include <octf/node/events/INodeEvent.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Node event filter
 *
 * This filter specifies which node event is enabled or disabled
 */
class NodeEventFilter : public NonCopyable {
public:
    NodeEventFilter()
            : NonCopyable()
            , m_filter() {}

    virtual ~NodeEventFilter() = default;

    /**
     * Enables specified event type in filter
     *
     * @tparam T Event type to be enabled
     *
     * @note T Must inherit by INodeEvent base class.
     */
    template <typename T>
    void enable() {
        static_assert(std::is_base_of<INodeEvent, T>(),
                      "T must inherit by INodeEvent");

        m_filter.insert(std::type_index(typeid(T)));
    }

    /**
     * Disables specified event type in filter
     *
     * @tparam T Event type to be disabled
     *
     * @note T Must inherit by INodeEvent base class.
     */
    template <typename T>
    void disable() {
        static_assert(std::is_base_of<INodeEvent, T>(),
                      "T must inherit by INodeEvent");

        m_filter.erase(std::type_index(typeid(T)));
    }

    /**
     * @brief Checks if specified event is enabled
     *
     * @param event Event to be checked if it is enabled in filter
     *
     * @retval true Event is enabled
     * @retval false Event is disabled
     */
    bool isEnabled(const INodeEvent &event) const {
        if (m_filter.find(std::type_index(typeid(event))) != m_filter.end()) {
            return true;
        } else {
            return false;
        }
    }

private:
    std::unordered_set<std::type_index> m_filter;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_EVENTS_NODEEVENTFILTER_H
