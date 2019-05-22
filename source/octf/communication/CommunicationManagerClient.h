/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERCLIENT_H
#define SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERCLIENT_H

#include <google/protobuf/service.h>

#include <octf/communication/ICommunicationManager.h>
#include <octf/communication/RpcChannelImpl.h>
#include <octf/node/INode.h>
#include <octf/socket/ISocketListener.h>
#include <octf/utils/Semaphore.h>

namespace octf {

class SocketManager;
class SocketConfig;
class IClientEventHandler;

/**
 * @brief Communication manager which maintains client communication
 *
 * This utility is used by node and it provides possibility of starting
 * client socket. Upon new incoming connection the manager creates
 * for it ConnectionContext. Also manager is responsible for destroying
 * connection context when the connection expire.
 */
class CommunicationManagerClient : public ICommunicationManager {
public:
    CommunicationManagerClient(std::unique_ptr<IClientEventHandler> handler);
    virtual ~CommunicationManagerClient();

    /**
     * @brief Start client socket
     *
     * @param cnfg Socket configuration
     */
    void startSocket(const SocketConfig &cnfg);

    /**
     * @brief Shutdown all opened sockets and connections
     */
    void shutdown();

    const RpcProxyShRef &getRpcProxy() {
        return m_rpcProxy;
    }

private:
    void onConnection(const SocketConnectionShRef &conn) override;

    void onConnectionExpiration(ConnectionContextShRef &&context) override;

    void handleServerMethod(const MethodShRef &method) override;

private:
    /**
     * Socket manager responsible for maintaining client socket
     */
    std::unique_ptr<SocketManager> m_socketMngr;

    /**
     * RPC proxy
     */
    RpcProxyShRef m_rpcProxy;

    /**
     * Active connection context
     */
    ConnectionContextShRef m_connCntx;

    /**
     * Lock for active connection context
     */
    std::mutex m_lock;

    /**
     * Handler of asynchronous signals like onAttach, onDetach,
     * or others events (e.g. command's progress).
     */
    std::unique_ptr<IClientEventHandler> m_eventHndlr;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERCLIENT_H
