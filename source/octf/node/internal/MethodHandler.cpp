/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/node/internal/MethodHandler.h>

#include <octf/communication/Method.h>

namespace octf {

MethodHandler::MethodHandler(INode *owner)
        : NonCopyable()
        , IMethodHandler()
        , m_owner(owner) {}

void MethodHandler::handleMethod(const MethodShRef &method) {
    INode *node = NULL;

    // We have to have reference to child during method executing. It prevents
    // against destroying target child from different thread and removing
    // reference of this child while method is executed.
    NodeShRef nodeRef;

    if (method->getNodePath() == m_owner->getNodePath()) {
        node = m_owner;
    } else {
        // Method is dedicated for a child, find node
        nodeRef = getChildByPath(method->getNodePath());
        if (nodeRef) {
            node = nodeRef.get();
        }
    }

    if (!node) {
        // Node for given path does not exist
        method->fail("No such node");
        return;
    }

    // Get Interface
    auto interface = node->getInterface(method->getInterfaceId());
    if (!interface) {
        method->fail("No such interface");
        return;
    }

    // Prepare input and output parameters
    if (!method->prepareParameters(interface)) {
        // Cannot prepare input and output parameters, mostly because of
        // problem with parsing input
        method->fail("Cannot parse input");
        return;
    }

    // Finally call method
    interface->CallMethod(method->getMethodDescriptor(interface),
                          method->getRpcController(), method->getInput(),
                          method->getOutput(), method.get());
}

NodeShRef MethodHandler::getChildByPath(const NodePath &path) {
    if (path.begin() == path.end()) {
        // path is empty and cannot identify node
        return nullptr;
    }

    auto iter = path.begin();
    if (*iter != m_owner->getNodeId()) {
        // The first entry on path is different than owner
        return nullptr;
    }
    iter++;

    NodeShRef child;

    // Over rest items in path
    for (; iter != path.end(); iter++) {
        if (!child) {
            // Select first child
            child = m_owner->getChild(*iter);
        } else {
            // Select next child (nested child, child of child)
            child = child->getChild(*iter);
        }

        if (!child) {
            // No such child, wrong path
            return nullptr;
        }
    }

    return child;
}

}  // namespace octf
