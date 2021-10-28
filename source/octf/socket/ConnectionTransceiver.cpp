/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/socket/ConnectionTransceiver.h>

#include <string>
#include <octf/socket/IConnectionTransceiverListener.h>
#include <octf/socket/internal/Packetizer.h>

using namespace std;

namespace octf {

ConnectionTransceiver::ConnectionTransceiver(
        const SocketConnectionShRef &conn,
        IConnectionTransceiverListener *listener)
        : m_conn(conn)
        , m_listener(listener)
        , m_rxThread()
        , m_txLock()
        , m_txPacketizer(new Packetizer(conn)) {}

ConnectionTransceiver::~ConnectionTransceiver() {}

bool ConnectionTransceiver::sendDataPacket(const std::string &data) {
    lock_guard<mutex> lock(m_txLock);

    if (m_txPacketizer->sendPacket(data)) {
        return true;
    } else {
        // An error while sending packet, close connection
        m_conn->close();
        return false;
    }
}

void ConnectionTransceiver::run() {
    string data;
    Packetizer packetizer(m_conn);

    while (m_conn->isActive()) {
        if (packetizer.receivePacket(data)) {
            m_listener->onDataPacket(data);
        } else {
            // An error while receiving packet, close connection
            m_conn->close();
        }
    }

    // Notify about disconnection
    m_listener->onDisconnection();
}

void ConnectionTransceiver::activate() {
    m_rxThread = thread([this]() { run(); });
}

void ConnectionTransceiver::deactivate() {
    // Synchronize de-activation by using transmitting lock,
    // no need to add another one
    lock_guard<mutex> lock(m_txLock);

    if (m_conn) {
        m_conn->close();
    }

    if (m_rxThread.joinable()) {
        if (m_rxThread.get_id() != this_thread::get_id()) {
            m_rxThread.join();
        } else {
            // We are in the context of RX thread, and can not join here,
            // it has to be done outside of this RX thread
        }
    }
}

}  // namespace octf
