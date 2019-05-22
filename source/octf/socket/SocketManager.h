/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_SOCKETMANAGER_H
#define SOURCE_OCTF_SOCKET_SOCKETMANAGER_H

#include <memory>
#include <thread>
#include <octf/socket/SocketConfig.h>

namespace octf {

class ISocketListener;
class SocketManagerStateMachine;

/**
 * @brief Generic implementation of manager for client or server socket
 *
 * Main purpose of socket manager is running thread in background and informing
 * about new connection on given socket
 */
class SocketManager {
public:
    SocketManager(ISocketListener *listener, const SocketConfig &cnfg);
    virtual ~SocketManager();

    /**
     * @brief Activates manager
     *
     * Starts background thread, creates sockets, listens for connection,
     * etc...
     */
    void activate();

    /**
     * Deactivates manager
     *
     * Stops background thread and closes socket
     */
    void deactivate();

    /**
     * @brief Checks if manager is active
     *
     * @return Returns true if manager is active
     */
    bool isActive() const;

    /**
     * @brief Gets socket configuration
     *
     * @return Socket configuration
     */
    const SocketConfig &getSocketConfig() const;

private:
    void run();

private:
    /**
     * Socket manager state machine
     */
    std::unique_ptr<SocketManagerStateMachine> m_sm;

    /**
     * threads in the context of which manager (state machine) is run
     * and generates events to the listener
     */
    std::thread m_smThread;

    /**
     * Boolean value controlling running of state machine
     */
    bool m_active;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_SOCKETMANAGER_H
