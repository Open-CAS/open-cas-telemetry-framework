/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_NODEGENERIC_H
#define SOURCE_OCTF_NODE_NODEGENERIC_H
#include <mutex>
#include <thread>
#include <octf/node/INode.h>
#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Generic implementation of INode interface
 */
class NodeGeneric : public INode, private NonCopyable {
public:
    NodeGeneric(const NodeId &id);
    virtual ~NodeGeneric() = default;

    // Method made final to disallow unintentional overriding
    inline bool init() final {
        if (!initCommon()) {
            return false;
        }

        if (!initCustom()) {
            // Revert previous initCommon
            deinitCommon();
            return false;
        }

        return true;
    }

    // Method made final to disallow unintentional overriding
    inline void deinit() final {
        deinitCustom();
        deinitCommon();
    }

    bool initCommon() override;

    void deinitCommon() override;

    const NodeId &getNodeId() const override {
        return m_id;
    }

    const NodePath &getNodePath() const {
        return m_path;
    }

    bool hasChild(const NodeId &id) const override;

    bool getChildrenIdList(NodesIdList &children) const override;

    NodeShRef getChild(const NodeId &nodeId) override;

    bool removeChild(const NodeId &nodeId) override;

    bool hasInterface(const InterfaceId &id) const override;

    InterfaceShRef getInterface(const InterfaceId &id) override;

    bool getInterfacesIdList(InterfacesIdList &interfaces) override;

    bool removeInterface(const InterfaceId &id) override;

    /**
     * @brief Find and get specific interface type
     *
     * @tparam T Class type of interface to be found and returned
     *
     * @return Shared pointer to the interface
     *
     * @retval Valid shared pointer indicates success of operation
     *
     * @retval Null shared pointer indicates interface  unavailability
     */
    template <typename T>
    std::shared_ptr<T> findInterface() {
        try {
            InterfaceId id(T::descriptor());

            auto interface = std::dynamic_pointer_cast<T>(getInterface(id));

            if (interface) {
                return interface;
            }

            return nullptr;

        } catch (Exception &) {
            return nullptr;
        }
    }

    bool addInterface(InterfaceShRef interface) override;

    bool addChild(NodeShRef child) override;

    /**
     * @note NodeGeneric returns "null" implementation of INodeEventDispatcher.
     * The "null" implementation ignores all calls and no effect take place.
     */
    INodeEventDispatcher &getEventDispatcher() override;

private:
    /**
     * @note This method doesn't acquire lock on interfaces list. Caller has to
     * acquire appropriate lock for synchronization.
     */
    inline bool hasInterface_NoLock(const InterfaceId &id) const;

    /**
     * @note This method doesn't acquire lock on children list. Caller has to
     * acquire appropriate lock for synchronization.
     */
    inline bool hasChild_NoLock(const NodeId &id) const;

private:
    /**
     * @typedef Node list consisted of shared pointers to nodes
     */
    typedef std::list<NodeShRef> NodesList;

    /**
     * @typedef Interface list consisted of shared pointers to interfaces
     */
    typedef std::list<InterfaceShRef> InterfacesList;

    /**
     * Node ID
     */
    NodeId m_id;

    /**
     * Node path
     */
    NodePath m_path;

    /**
     * Interfaces List
     */
    InterfacesList m_interfaces;

    /**
     * Children node list
     */
    NodesList m_children;

    mutable std::mutex m_interfacesLock;
    mutable std::mutex m_nodesLock;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODEGENERIC_H
