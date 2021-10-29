/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_ICONNECTIONTRANSCEIVERLISTENER_H
#define SOURCE_OCTF_SOCKET_ICONNECTIONTRANSCEIVERLISTENER_H
#include <string>

namespace octf {

/**
 * @interface IConnectionTransceiverListener
 *
 * @brief Listener used by ConnectionTransceiver in order to signal about new
 * incoming data packet and connection termination
 */
class IConnectionTransceiverListener {
public:
    IConnectionTransceiverListener() {}
    virtual ~IConnectionTransceiverListener() {}

    /**
     * @brief On new data packet received
     *
     * @param data Received data
     */
    virtual void onDataPacket(const std::string &data) = 0;

    /**
     * @brief On disconnection event
     */
    virtual void onDisconnection() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_ICONNECTIONTRANSCEIVERLISTENER_H
