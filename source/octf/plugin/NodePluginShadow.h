/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_PLUGIN_NODEPLUGINSHADOW_H
#define SOURCE_OCTF_PLUGIN_NODEPLUGINSHADOW_H
#include <string>
#include <octf/node/NodeClient.h>

namespace octf {

enum class PluginState {
    UNDEFINED = 0,

    CONNECTED,
    DISCONNECTED,
    ERROR
};

/**
 * @class Base class for plugin client
 */
class NodePluginShadow : public NodeClient {
public:
    NodePluginShadow(const NodeId &id);
    virtual ~NodePluginShadow();

    // Final method, additional initialization required by derived classes
    // should be implemented in initCustom method
    bool initCommon() final;

    // Final method, additional initialization required by derived classes
    // should be implemented in initCustom method
    void deinitCommon() final;

    bool initCustom() override;

    void deinitCustom() override;

    INodeEventDispatcher &getEventDispatcher() override;

    inline const PluginState &getState() const {
        return m_state;
    }

    inline void setState(const PluginState &state) {
        m_state = state;
    }

    const std::string &getDescription() const;

    void setDescription(const std::string &desc);

    const std::string &getError() const;

    void setError(const std::string &error);

private:
    /**
     * Shared reference wrapping this instance of shadow plugin
     */
    std::shared_ptr<INode> m_wrapper;

    /**
     * node event dispatcher
     */
    std::unique_ptr<INodeEventDispatcher> m_dispatcher;

    PluginState m_state;
    std::string m_description;
    std::string m_error;
};

}  // namespace octf

#endif  // SOURCE_OCTF_PLUGIN_NODEPLUGINSHADOW_H
