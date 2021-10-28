/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINECLIENT_H
#define SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINECLIENT_H

#include <thread>
#include <octf/socket/ISocketClient.h>
#include <octf/socket/ISocketConnection.h>
#include <octf/socket/internal/SocketManagerStateMachine.h>

namespace octf {

class ISocketListener;

/**
 * @brief Utilities class which is able to work with client socket and generates
 * event through ISocketListener
 *
 * @note Only one connection is allowed (maintained) in the time
 *
 * @note This class is private component of socket source tree and intended
 * to be used by common library
 */
class SocketManagerStateMachineClient : public SocketManagerStateMachine {
public:
    /**
     * @param listener Listener which receives signals about new
     * incoming connection
     * @param cnfg Socket configuration
     */
    SocketManagerStateMachineClient(ISocketListener *listener,
                                    const SocketConfig &cnfg);
    virtual ~SocketManagerStateMachineClient();

    State init();

    State work();

    void close();

private:
    /**
     * Client Socket
     */
    SocketClientShRef m_clnt;

    /**
     * Current connection
     */
    SocketConnectionShRef m_conn;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINECLIENT_H
