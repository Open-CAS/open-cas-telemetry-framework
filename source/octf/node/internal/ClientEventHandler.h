/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_INTERNAL_CLIENTEVENTHANDLER_H
#define SOURCE_OCTF_NODE_INTERNAL_CLIENTEVENTHANDLER_H

#include <functional>
#include <octf/communication/internal/IClientEventHandler.h>

namespace octf {

class INode;

/**
 * @class Client handler implementation
 *
 * This handler is supposed to be used for catching clients events and forward
 * them into root node and its children.
 */
class ClientEventHandler : public IClientEventHandler {
public:
    ClientEventHandler(INode *owner);
    virtual ~ClientEventHandler() = default;

    void onAttach() override;

    void onDetach() override;

private:
    /**
     * Owner which handles events
     */
    INode *m_owner;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_INTERNAL_CLIENTEVENTHANDLER_H
