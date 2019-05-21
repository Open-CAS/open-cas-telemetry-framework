/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINESERVER_H
#define SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINESERVER_H

#include <octf/socket/ISocketServer.h>
#include <octf/socket/internal/SocketManagerStateMachine.h>

namespace octf {

class ISocketListener;

/**
 * @brief Utilities class which is able to work with server socket and generates
 * event through ISocketListener
 *
 * @note This class is private component of socket source tree and intended
 * to be used by common library
 */
class SocketManagerStateMachineServer : public SocketManagerStateMachine {
public:
    /**
     * @param listener Listener which receives signals about new
     * incoming connection
     * @param cnfg Socket configuration
     */
    SocketManagerStateMachineServer(ISocketListener *listener,
                                    const SocketConfig &cnfg);
    virtual ~SocketManagerStateMachineServer();

private:
    State init() override;

    State work() override;

    void close() override;

private:
    /**
     * Server socket
     */
    SocketServerShRef m_srv;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINESERVER_H
