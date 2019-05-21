/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/socket/internal/SocketManagerStateMachineServer.h>

#include <octf/socket/ISocketListener.h>
#include <octf/socket/SocketFactory.h>
#include <octf/socket/internal/SocketManagerStateMachine.h>
#include <octf/utils/Exception.h>

using namespace std;

namespace octf {

SocketManagerStateMachineServer::SocketManagerStateMachineServer(
        ISocketListener *listener,
        const SocketConfig &cnfg)
        : SocketManagerStateMachine(listener, cnfg)
        , m_srv() {
    if (cnfg.type != SocketType::Server) {
        throw Exception("Only server socket type is accepted.");
    }
}

SocketManagerStateMachineServer::~SocketManagerStateMachineServer() {
    close();
}

SocketManagerStateMachine::State SocketManagerStateMachineServer::init() {
    if (m_srv) {
        // Reset previously used socket server, if exist
        m_srv.reset();
    }

    m_srv = SocketFactory::createServer(getSocketConfig().address,
                                        getSocketConfig().implementation);
    if (m_srv) {
        // Try open socket
        if (!m_srv->open()) {
            // Open failed, reset server
            m_srv.reset();
        }
    }

    if (m_srv) {
        // Server initialized correctly
        return State::Working;
    } else {
        // Error, switch to idle state for a while
        return State::Idle;
    }
}

SocketManagerStateMachine::State SocketManagerStateMachineServer::work() {
    SocketConnectionShRef conn = m_srv->listen();
    if (conn) {
        getListener()->onConnection(conn);
    }

    if (!m_srv->isActive()) {
        // server has come inactive

        // Reset server
        m_srv.reset();

        // Move to idle state for a while
        return State::Idle;
    }

    return State::Working;
}

void SocketManagerStateMachineServer::close() {
    if (m_srv) {
        m_srv->close();
    }
}

}  // namespace octf
