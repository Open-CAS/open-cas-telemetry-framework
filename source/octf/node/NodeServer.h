/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_NODE_NODESERVER_H
#define SOURCE_OCTF_NODE_NODESERVER_H
#include <memory>
#include <octf/node/NodeBase.h>

namespace octf {

class CommunicationManagerServer;
class MethodHandler;

/**
 * Base node class intends to be used by server nodes (like plugins)
 */
class NodeServer : public NodeBase {
public:
    NodeServer(const NodeId &id);
    virtual ~NodeServer();

    bool initCommon() override;

    void deinitCommon() override;

    bool initCustom() override;

    void deinitCustom() override;

private:
    /**
     * Communication manager responsible for communication (e.g. opening
     * server socket)
     */
    std::unique_ptr<CommunicationManagerServer> m_commMngr;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODESERVER_H
