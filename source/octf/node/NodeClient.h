/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_NODECLIENT_H
#define SOURCE_OCTF_NODE_NODECLIENT_H
#include <octf/node/NodeShadow.h>

namespace octf {

class CommunicationManagerClient;

/**
 * Base node class intends to provide communication for client
 */
class NodeClient : public NodeShadow {
public:
    NodeClient(const NodeId &id);

    virtual ~NodeClient();

    bool initCommon() override;

    void deinitCommon() override;

    bool initCustom() override;

    void deinitCustom() override;

    virtual const RpcProxyShRef &getRpcProxy();

private:
    /**
     * Communication manager responsible for instance for starting client socket
     */
    std::unique_ptr<CommunicationManagerClient> m_commMngr;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODECLIENT_H
