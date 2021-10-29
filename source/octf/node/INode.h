/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_INODE_H
#define SOURCE_OCTF_NODE_INODE_H

#include <google/protobuf/service.h>
#include <list>
#include <memory>
#include <string>
#include <octf/interface/InterfaceId.h>
#include <octf/node/NodeId.h>
#include <octf/node/events/INodeEventDispatcher.h>

namespace octf {

class IInterface;
class INode;

// Typedefs

// Smart pointers

/**
 * @typedef Shared pointer to INode
 */
typedef std::shared_ptr<INode> NodeShRef;

/**
 * @typedef Unique pointer to INode
 */
typedef std::unique_ptr<INode> NodeUniqueRef;

/**
 * @typedef Weak pointer to INode
 */
typedef std::weak_ptr<INode> NodeWeakRef;

/**
 * @typedf Shared pointer to google protobuf Service (interface)
 */
typedef std::shared_ptr<google::protobuf::Service> InterfaceShRef;

// Lists

/**
 * List of Nodes ID
 */
typedef std::list<NodeId> NodesIdList;

/**
 * List of Interfaces
 */
typedef std::list<InterfaceId> InterfacesIdList;

/**
 * @brief Common interface for all types of nodes.
 */
class INode {
public:
    virtual ~INode() = default;

    /**
     * @brief Gets the node ID.
     *
     * @return NodeId of the node.
     */
    const virtual NodeId &getNodeId() const = 0;

    /**
     * @brief Gets the node path.
     *
     * @return The node path.
     */
    const virtual NodePath &getNodePath() const = 0;

    /**
     * @brief Performs complete initialization of a node.
     *
     * @note This method calls initCommon() and initCustom()
     *
     * @return Status of initialization.
     * True - on successful initialization.
     * False - if any error occurred.
     */
    virtual bool init() = 0;

    /**
     * @brief Performs complete deinitialization and cleaning up of a node.
     *
     * @note This method calls deinitCustom() and deinitCommon()
     */
    virtual void deinit() = 0;

    /**
     * @brief Initializes node as a fully functional node. This method is
     * finalized at a certain level of class hierarchy. To extend the
     * initializing functionality, initCustom should be used.
     *
     * @note This method is called by init()
     *
     * @return Status of initialization.
     * True - on successful initialization.
     * False - if any error occurred.
     */
    virtual bool initCommon() = 0;

    /**
     * @brief De-initializes and cleanups node. This method is
     * finalized at a certain level of class hierarchy. To extend the
     * initializing functionality, deinitCustom should be used.
     *
     * @note This method is called by deinit()
     */
    virtual void deinitCommon() = 0;

    /**
     * @brief Performs any additional initialization. Override this
     * method when inheriting from Node classes to perform any additional
     * initialization steps.
     *
     * @note This method is called by init()
     */
    virtual bool initCustom() = 0;

    /**
     * @brief Performs any additional cleanup. Override this
     * method when inheriting from Node classes to perform any additional
     * initialization steps.
     *
     * @note This method is called by deinit()
     */
    virtual void deinitCustom() = 0;

    /**
     * @brief Checks if the child is on the Node's list.
     *
     * @param id Unique ID of the child node.
     *
     * @return True if child with specified Node ID is present on
     * the Node's list of children. False otherwise.
     */
    virtual bool hasChild(const NodeId &id) const = 0;

    /**
     * @brief Gets the child node identified by nodeID.
     *
     * @param nodeId Unique ID of child node.
     *
     * @return Shared pointer to the child node with specified ID.
     * Nullptr if such node does not exist on the list.
     *
     */
    virtual NodeShRef getChild(const NodeId &nodeId) = 0;

    /**
     * @brief Gets list of children nodes.
     *
     * @param[out] children List of all childrens' NodeId-s.
     *
     * @return Status of operation.
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool getChildrenIdList(NodesIdList &children) const = 0;

    /**
     * @brief Removes a child node from the list.
     *
     * @param childNodeId NodeId of the node to be removed.
     *
     * @return Status of operation
     * True - on success.
     * False - if any error occurred or child was not on the list.
     */
    virtual bool removeChild(const NodeId &childNodeId) = 0;

    /**
     * @brief Checks if specified interface is on the Node's list.
     *
     * @param id Unique ID of the interface.
     *
     * @return True if interface with specified ID is present on
     * the Node's list of interfaces. False otherwise.
     */
    virtual bool hasInterface(const InterfaceId &id) const = 0;

    /**
     * @brief Gets the interface identified by name.
     *
     * @param id Unique ID of the interface.
     *
     * @return Shared pointer to the interface object with specified name.
     * Nullptr if such interface does not exist on the list.
     */
    virtual InterfaceShRef getInterface(const InterfaceId &id) = 0;

    /**
     * @brief Gets list of interfaces.
     *
     * @param[out] interfaces List of all interfaces' InterfaceId-s.
     *
     * @return Status of operation.
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool getInterfacesIdList(InterfacesIdList &interfaces) = 0;

    /**
     * @brief Removes an interface from the list.
     *
     * @param id InterfaceId of the interface to be removed.
     *
     * @return Status of operation
     * True - on success.
     * False - if any error occurred or interface was not on the list.
     */
    virtual bool removeInterface(const InterfaceId &id) = 0;

    /**
     * @brief Adds child to the children list
     *
     * @param child Shared pointer of child to be added
     *
     * @return Status of operation.
     * @retval True - on successful operation.
     * @retval False - if any error occurred. For instance when child with
     * the same ID already exist at the children list.
     */
    virtual bool addChild(NodeShRef child) = 0;

    /**
     * @brief Adds interface to the interface list
     *
     * @param interface Shared pointer of interface to be added
     *
     * @return Status of operation.
     * @retval True - on successful operation.
     * @retval False - if any error occurred. For instance when interface with
     * the same ID already exist at the interface list.
     */
    virtual bool addInterface(InterfaceShRef interface) = 0;

    /**
     * @brief Gets node event dispatcher
     *
     * @return Node event dispatcher
     */
    virtual INodeEventDispatcher &getEventDispatcher() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_INODE_H
