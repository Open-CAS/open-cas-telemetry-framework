/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_CONNECTIONTRANSCEIVER_H
#define SOURCE_OCTF_SOCKET_CONNECTIONTRANSCEIVER_H

#include <mutex>
#include <string>
#include <thread>

#include <octf/socket/ISocketConnection.h>

namespace octf {

struct SocketConfig;
class IConnectionTransceiverListener;
class Packetizer;

/**
 * @brief Connection Transceiver
 *
 * Utility class which handles connection in separate thread in order to
 * receive data and inform about connection closure. In addition it synchronizes
 * sent packets over this connection
 */
class ConnectionTransceiver {
public:
    /**
     * @param conn Connection which will be handled (processed)
     * @param listener Listener which waits on events like: data receiving,
     * connection closure
     */
    ConnectionTransceiver(const SocketConnectionShRef &conn,
                          IConnectionTransceiverListener *listener);
    virtual ~ConnectionTransceiver();

    /**
     * @brief Activates transceiver
     */
    void activate();

    /**
     * @brief Deactivates transceiver
     */
    void deactivate();

    /**
     * @brief Sends data as a packet over this transceiver
     *
     * @param data Data to be sent over connection
     *
     * @return Operation result, if success returns true
     */
    bool sendDataPacket(const std::string &data);

private:
    /**
     * @brief It is thread main loop
     */
    void run();

private:
    /**
     * Connection to be handled
     */
    SocketConnectionShRef m_conn;

    /**
     * Socket listener
     */
    IConnectionTransceiverListener *m_listener;

    /**
     * Thread in the context of which socket listener will receive
     * data and information about connection disconnection
     */
    std::thread m_rxThread;

    /**
     * Lock for synchronization transmitting operation
     */
    std::mutex m_txLock;

    /**
     * Transmitting packetizer
     */
    std::unique_ptr<Packetizer> m_txPacketizer;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_CONNECTIONTRANSCEIVER_H
