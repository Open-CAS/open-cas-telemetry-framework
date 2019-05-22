/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/communication/CommunicationManagerServer.h>

#include <octf/communication/Method.h>
#include <octf/communication/internal/ConnectionContext.h>
#include <octf/communication/internal/ConnectionContextDeleter.h>
#include <octf/socket/SocketConfig.h>
#include <octf/socket/SocketManager.h>
#include <octf/utils/Exception.h>

using namespace std;

namespace octf {

CommunicationManagerServer::CommunicationManagerServer(
        std::unique_ptr<IMethodHandler> handler)
        : ICommunicationManager()
        , m_deleter(std::unique_ptr<ConnectionContextDeleter>(
                  new ConnectionContextDeleter()))
        , m_socketMngr()
        , m_activeCntxlist()
        , m_lock()
        , m_methodHndlr(std::move(handler)) {}

CommunicationManagerServer::~CommunicationManagerServer() {
    shutdown();
}

void CommunicationManagerServer::startSocket(const SocketConfig &cnfg) {
    if (m_socketMngr) {
        // Improper program flow, to start new socket, first call shutdown
        throw Exception("Socket already started.");
    }

    if (cnfg.type != SocketType::Server) {
        throw Exception("Expecting server socket configuration.");
    }

    m_socketMngr = unique_ptr<SocketManager>(new SocketManager(this, cnfg));
    m_socketMngr->activate();
}

void CommunicationManagerServer::shutdown() {
    if (m_socketMngr) {
        // Stop all sockets
        m_socketMngr->deactivate();
        m_socketMngr.reset();
        // No new connection starting once socket stopped
    }

    ConnectionContextShRef context;

    // Reset all contexts
    do {
        if (context) {
            context.reset();
        }

        {
            lock_guard<mutex> lock(m_lock);

            if (m_activeCntxlist.size()) {
                // List not empty pick from front
                context = *m_activeCntxlist.begin();
                m_activeCntxlist.pop_front();
            }
        }
    } while (context);
}

void CommunicationManagerServer::onConnection(
        const SocketConnectionShRef &conn) {
    auto connCntx = make_shared<ConnectionContext>(this, conn);

    {
        lock_guard<mutex> lock(m_lock);
        m_activeCntxlist.push_back(connCntx);
    }

    connCntx->activate();
}

void CommunicationManagerServer::onConnectionExpiration(
        ConnectionContextShRef &&context) {
    lock_guard<mutex> lock(m_lock);

    auto iter = m_activeCntxlist.begin();
    for (; iter != m_activeCntxlist.end(); iter++) {
        if (*iter == context) {
            m_activeCntxlist.erase(iter);

            // move context into deleter
            m_deleter->addConnectionContext(std::move(context));
            break;
        }
    }
}

void CommunicationManagerServer::handleServerMethod(const MethodShRef &method) {
    if (!m_methodHndlr) {
        // No method handler, fail method
        method->fail("No method handler");
        return;
    }

    m_methodHndlr->handleMethod(method);
}

}  // namespace octf
