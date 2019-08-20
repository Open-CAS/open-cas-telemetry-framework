/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERSERVER_H
#define SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERSERVER_H

#include <google/protobuf/service.h>
#include <list>
#include <memory>
#include <mutex>
#include <string>

#include <octf/communication/ICommunicationManager.h>
#include <octf/node/INode.h>
#include <octf/socket/ISocketListener.h>

namespace octf {

struct SocketConfig;
class SocketManager;
class ConnectionContextDeleter;
class IMethodHandler;

/**
 * @brief Communication manager which maintains server communication
 *
 * This utility is used by node and it provides possibility of starting
 * server socket. Upon new incoming connection, the manager creates
 * for it ConnectionContext. Also manager is responsible for destroying
 * connection context when the connection expires.
 */
class CommunicationManagerServer : public ICommunicationManager {
public:
    CommunicationManagerServer(std::unique_ptr<IMethodHandler> handler);
    virtual ~CommunicationManagerServer();

    /**
     * @brief Start server socket
     *
     * @param cnfg Socket configuration
     */
    void startSocket(const SocketConfig &cnfg);

    /**
     * @brief Shutdown all opened sockets and connections
     */
    void shutdown();

private:
    void onConnection(const SocketConnectionShRef &conn) override;

    void onConnectionExpiration(ConnectionContextShRef &&context) override;

    void handleServerMethod(const MethodShRef &method) override;

private:
    /**
     * Deleter for inactive connection contexts
     */
    std::unique_ptr<ConnectionContextDeleter> m_deleter;

    /**
     * Socket server manager responsible for maintaining server socket
     */
    std::unique_ptr<SocketManager> m_socketMngr;

    /**
     * List of active connection contexts
     */
    std::list<ConnectionContextShRef> m_activeCntxlist;

    /**
     * Lock for list of active connection contexts
     */
    std::mutex m_lock;

    /**
     * Handler of method to be executed by service or plugin.
     */
    std::unique_ptr<IMethodHandler> m_methodHndlr;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_COMMUNICATIONMANAGERSERVER_H
