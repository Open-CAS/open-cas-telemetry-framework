/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/node/NodeGeneric.h>

#include <algorithm>
#include <memory>
#include <octf/interface/InterfaceIdentificationImpl.h>
#include <octf/node/internal/NodeEventDispatcherNull.h>
#include <octf/proto/InterfaceIdentification.pb.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>

namespace octf {

static InterfaceId getInterfaceId(InterfaceShRef interface) {
    return InterfaceId(interface->GetDescriptor());
}

// operators required for calling find() function on list
static bool operator==(const NodeShRef node, const NodeId id) {
    return (node->getNodeId() == id);
}

static bool operator==(const InterfaceShRef interface, const InterfaceId id) {
    return (getInterfaceId(interface) == id);
}

NodeGeneric::NodeGeneric(const NodeId &id)
        : m_id(id)
        , m_path()
        , m_interfaces()
        , m_children() {
    m_path.push_back(m_id);
}

bool NodeGeneric::getChildrenIdList(NodesIdList &children) const {
    children.clear();

    try {
        std::lock_guard<std::mutex> lock(m_nodesLock);
        for (auto node : m_children) {
            children.push_back(node->getNodeId());
        }
        return true;
    } catch (Exception &ex) {
        children.clear();
        return false;
    }
}

NodeShRef NodeGeneric::getChild(const NodeId &nodeId) {
    std::lock_guard<std::mutex> lock(m_nodesLock);
    auto nodeIter = std::find(m_children.begin(), m_children.end(), nodeId);
    if (nodeIter != m_children.end()) {
        return *nodeIter;
    } else {
        return nullptr;
    }
}

bool NodeGeneric::removeChild(const NodeId &nodeId) {
    std::lock_guard<std::mutex> lock(m_nodesLock);
    auto nodeIter = std::find(m_children.begin(), m_children.end(), nodeId);
    if (nodeIter != m_children.end()) {
        m_children.erase(nodeIter);
        return true;
    } else {
        return false;
    }
}

bool NodeGeneric::removeInterface(const InterfaceId &id) {
    std::lock_guard<std::mutex> lock(m_interfacesLock);
    for (auto iter = m_interfaces.begin(); iter != m_interfaces.end(); ++iter) {
        if (getInterfaceId(*iter) == id) {
            m_interfaces.erase(iter);
            return true;
        }
    }

    return false;
}

bool NodeGeneric::hasInterface_NoLock(const InterfaceId &id) const {
    auto interfaceIter =
            std::find(m_interfaces.begin(), m_interfaces.end(), id);
    if (interfaceIter != m_interfaces.end()) {
        return true;
    } else {
        return false;
    }
}

bool NodeGeneric::hasInterface(const InterfaceId &id) const {
    std::lock_guard<std::mutex> lock(m_interfacesLock);

    return hasInterface_NoLock(id);
}

bool NodeGeneric::hasChild_NoLock(const NodeId &id) const {
    auto nodeIter = std::find(m_children.begin(), m_children.end(), id);

    if (nodeIter != m_children.end()) {
        return true;
    } else {
        return false;
    }
}

bool NodeGeneric::hasChild(const NodeId &id) const {
    std::lock_guard<std::mutex> lock(m_nodesLock);

    return hasChild_NoLock(id);
}

bool NodeGeneric::getInterfacesIdList(InterfacesIdList &interfaces) {
    interfaces.clear();

    try {
        std::lock_guard<std::mutex> lock(m_interfacesLock);
        for (auto iter : m_interfaces) {
            interfaces.push_back(getInterfaceId(iter));
        }
        return true;
    } catch (Exception &ex) {
        interfaces.clear();
        return false;
    }
}

InterfaceShRef NodeGeneric::getInterface(const InterfaceId &id) {
    std::lock_guard<std::mutex> lock(m_interfacesLock);
    for (auto iter : m_interfaces) {
        if (getInterfaceId(iter) == id) {
            return iter;
        }
    }
    return nullptr;
}

bool NodeGeneric::addInterface(InterfaceShRef interface) {
    InterfaceId id(interface->GetDescriptor());

    std::lock_guard<std::mutex> lock(m_interfacesLock);

    if (hasInterface_NoLock(id)) {
        return false;
    }

    m_interfaces.push_back(interface);

    return true;
}

bool NodeGeneric::addChild(NodeShRef child) {
    // try to cast into generic child
    auto gChild = std::dynamic_pointer_cast<NodeGeneric>(child);
    if (gChild) {
        // Set path
        gChild->m_path = this->m_path;
        gChild->m_path.push_back(gChild->getNodeId());
    } else {
        // It's different type of node. Parent (this) is adding different type
        // of child (not node generic). We don't have possibility, we don't
        // know implementation of path for that kind of child. So path has
        // to be handled by that node on its own or parent.
    }

    std::lock_guard<std::mutex> lock(m_nodesLock);

    if (hasChild_NoLock(child->getNodeId())) {
        return false;
    }

    m_children.push_back(child);

    return true;
}

bool NodeGeneric::initCommon() {
    return true;
}

void NodeGeneric::deinitCommon() {}

INodeEventDispatcher &NodeGeneric::getEventDispatcher() {
    static NodeEventDispatcherNull nullDispatcher;

    return nullDispatcher;
}

}  // namespace octf
