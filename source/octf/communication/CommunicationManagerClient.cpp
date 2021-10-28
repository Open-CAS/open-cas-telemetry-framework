/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/communication/CommunicationManagerClient.h>

#include <octf/communication/Method.h>
#include <octf/communication/internal/ConnectionContext.h>
#include <octf/communication/internal/ConnectionContextDeleter.h>
#include <octf/communication/internal/IClientEventHandler.h>
#include <octf/communication/internal/RpcProxy.h>
#include <octf/socket/SocketConfig.h>
#include <octf/socket/SocketManager.h>
#include <octf/utils/Exception.h>

namespace octf {

CommunicationManagerClient::CommunicationManagerClient(
        std::unique_ptr<IClientEventHandler> handler)
        : ICommunicationManager()
        , m_socketMngr()
        , m_rpcProxy(std::make_shared<RpcProxy>())
        , m_connCntx()
        , m_lock()
        , m_eventHndlr(std::move(handler)) {}

CommunicationManagerClient::~CommunicationManagerClient() {
    if (m_socketMngr) {
        m_socketMngr->deactivate();
    }
}

void CommunicationManagerClient::startSocket(const SocketConfig &cnfg) {
    if (m_socketMngr) {
        // Improper program flow, to start new socket, first call shutdown
        throw Exception("Socket already started.");
    }

    if (cnfg.type != SocketType::Client) {
        throw Exception("Expecting client socket configuration.");
    }

    m_socketMngr =
            std::unique_ptr<SocketManager>(new SocketManager(this, cnfg));
    m_socketMngr->activate();
}

void CommunicationManagerClient::shutdown() {
    if (m_socketMngr) {
        // Stop socket manager, if stopped then no new connection
        m_socketMngr->deactivate();
        m_socketMngr.reset();
    }

    {
        std::lock_guard<std::mutex> lock(m_lock);

        // Clear connection in RPC proxy
        m_rpcProxy->clearConnectionContext();
    }
}

void CommunicationManagerClient::onConnection(
        const SocketConnectionShRef &conn) {
    std::lock_guard<std::mutex> lock(m_lock);

    // Set new connection context or exchange previous one
    m_connCntx = std::make_shared<ConnectionContext>(this, conn);

    // Update connection context in RPC proxy
    m_rpcProxy->setConnectionContext(m_connCntx);

    if (m_eventHndlr) {
        m_eventHndlr->onAttach();
    }

    m_connCntx->activate();
}

void CommunicationManagerClient::onConnectionExpiration(
        ConnectionContextShRef &&context) {
    std::lock_guard<std::mutex> lock(m_lock);

    // Client manager has only one connection, so it's sanity check if
    // expired context is the same as this manager saved
    if (m_connCntx != context) {
        throw Exception("Inconsistent connection context.");
    }

    // Clear connections in RPC proxy
    m_rpcProxy->clearConnectionContext();

    if (m_eventHndlr) {
        m_eventHndlr->onDetach();
    }
}

void CommunicationManagerClient::handleServerMethod(const MethodShRef &method) {
    method->fail("Method execution not supported");
}

}  // namespace octf
