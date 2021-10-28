/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/socket/internal/SocketManagerStateMachineClient.h>

#include <octf/socket/ISocketListener.h>
#include <octf/socket/SocketFactory.h>
#include <octf/utils/Exception.h>

using namespace std;

namespace octf {

SocketManagerStateMachineClient::SocketManagerStateMachineClient(
        ISocketListener *listener,
        const SocketConfig &cnfg)
        : SocketManagerStateMachine(listener, cnfg)
        , m_clnt()
        , m_conn() {
    if (cnfg.type != SocketType::Client) {
        throw Exception("Only client socket type is accepted.");
    }
}

SocketManagerStateMachineClient::~SocketManagerStateMachineClient() {
    close();
}

SocketManagerStateMachine::State SocketManagerStateMachineClient::init() {
    if (m_conn) {
        m_conn->close();
        m_conn.reset();
    }

    if (m_clnt) {
        // Reset previously used socket Client, if exist
        m_clnt.reset();
    }

    m_clnt = SocketFactory::createClient(getSocketConfig().address,
                                         getSocketConfig().implementation);
    if (m_clnt) {
        // Try connect
        m_conn = m_clnt->connect();

        if (m_conn) {
            // Connection is active, inform about this
            getListener()->onConnection(m_conn);
        }
    }

    if (m_conn) {
        // Client initialized correctly
        return State::Working;
    } else {
        // Error, switch to idle state for a while
        return State::Idle;
    }
}

SocketManagerStateMachine::State SocketManagerStateMachineClient::work() {
    while (m_conn->isActive()) {
        this_thread::sleep_for(getIdleTimeout());
    }

    return State::Idle;
}

void SocketManagerStateMachineClient::close() {
    if (m_conn) {
        m_conn->close();
    }
}

}  // namespace octf
