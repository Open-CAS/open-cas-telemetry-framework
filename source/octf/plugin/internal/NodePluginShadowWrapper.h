/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_PLUGIN_INTERNAL_NODEPLUGINSHADOWWRAPPER_H
#define SOURCE_OCTF_PLUGIN_INTERNAL_NODEPLUGINSHADOWWRAPPER_H

#include <octf/node/INode.h>
#include <octf/utils/Exception.h>

namespace octf {

class NodePluginShadow;

/**
 * @class This class is just a wrapper of node plugin shadow
 */
class NodePluginShadowWrapper : public INode {
public:
    /**
     * @param Node Plugin shadow target which will be wrapped by this class
     */
    NodePluginShadowWrapper(NodePluginShadow *target)
            : INode()
            , m_target(target) {}

    virtual ~NodePluginShadowWrapper() = default;

    const inline NodeId &getNodeId() const final {
        return m_target->getNodeId();
    }

    const inline NodePath &getNodePath() const final {
        return m_target->getNodePath();
    }

    inline bool init() final {
        // TODO (mbarczak) Consider if throws in initialization methods are
        // corrected
        throw Exception("Initialization cannot be called on wrapper");
    }

    inline void deinit() final {
        throw Exception("De-initialization cannot be called on wrapper");
    }

    inline bool initCommon() final {
        throw Exception("Common initialization cannot be called on wrapper");
    }

    inline void deinitCommon() final {
        throw Exception("Common de-initialization cannot be called on wrapper");
    }

    inline bool initCustom() final {
        throw Exception("Custom initialization cannot be called on wrapper");
    }

    inline void deinitCustom() final {
        throw Exception("Custom de-initialization cannot be called on wrapper");
    }

    inline bool hasChild(const NodeId &id) const final {
        return m_target->hasChild(id);
    }

    inline NodeShRef getChild(const NodeId &nodeId) final {
        return m_target->getChild(nodeId);
    }

    inline bool getChildrenIdList(NodesIdList &children) const final {
        return m_target->getChildrenIdList(children);
    }

    inline bool removeChild(const NodeId &childNodeId) final {
        return m_target->removeChild(childNodeId);
    }

    inline bool hasInterface(const InterfaceId &id) const final {
        return m_target->hasInterface(id);
    }

    inline InterfaceShRef getInterface(const InterfaceId &id) final {
        return m_target->getInterface(id);
    }

    inline bool getInterfacesIdList(InterfacesIdList &interfaces) final {
        return m_target->getInterfacesIdList(interfaces);
    }

    inline bool removeInterface(const InterfaceId &id) final {
        return m_target->removeInterface(id);
    }

    inline bool addChild(NodeShRef child) final {
        return m_target->addChild(child);
    }

    inline bool addInterface(InterfaceShRef interface) final {
        return m_target->addInterface(interface);
    }

    inline INodeEventDispatcher &getEventDispatcher() final {
        return m_target->getEventDispatcher();
    }

private:
    INode *m_target;
};

}  // namespace octf

#endif  // SOURCE_OCTF_PLUGIN_INTERNAL_NODEPLUGINSHADOWWRAPPER_H
