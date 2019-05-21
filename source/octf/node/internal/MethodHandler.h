/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_INTERNAL_METHODHANDLER_H
#define SOURCE_OCTF_NODE_INTERNAL_METHODHANDLER_H
#include <octf/communication/IMethodHandler.h>
#include <octf/node/INode.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Class intends to handle communication events (e.g. handle method
 * to be executed on given node and give interface)
 */
class MethodHandler : public NonCopyable, public IMethodHandler {
public:
    MethodHandler(INode *owner);
    virtual ~MethodHandler() = default;

    void handleMethod(const MethodShRef &method) override;

private:
    /**
     * @brief Gets node by path
     *
     * @param path Node path
     *
     * @return Node for requested path
     * @retval valid node sheared pointer reference, node was found by path
     * @retval empty node shared reference - node not found by path
     */
    NodeShRef getChildByPath(const NodePath &path);

private:
    /**
     * Owner which handles events (method requests)
     */
    INode *m_owner;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_INTERNAL_METHODHANDLER_H
