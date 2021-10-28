/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINE_H
#define SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINE_H
#include <chrono>

#include <octf/socket/SocketConfig.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

class ISocketListener;

/**
 * @brief Basic class with state machine for socket
 *
 * This class creates thread in which state machine of socket is run.
 * The main purpose of this class is providing socket working all the time.
 *
 * @note This class is private component of socket source tree and intended
 * to be used by common library
 */
class SocketManagerStateMachine : private NonCopyable {
public:
    SocketManagerStateMachine(ISocketListener *listener,
                              const SocketConfig &cnfg);
    virtual ~SocketManagerStateMachine();

    /**
     * @brief State machine enumerator
     */
    enum class State { Idle, Initializing, Working };

    /**
     * @brief Implementation of Idle state
     *
     * @return Next state to be processed by state machine
     */
    virtual State idle();

    /**
     * @brief Implementation of Initializing state
     *
     * @return Next state to be processed by state machine
     */
    virtual State init() = 0;

    /**
     * @brief Implementation of Working state
     *
     * @return Next state to be processed by state machine
     */
    virtual State work() = 0;

    /**
     * @brief Close sockets upon cleanup
     */
    virtual void close() = 0;

    /**
     * @brief Gets socket configuration
     *
     * @return Socket configuration
     */
    const SocketConfig &getSocketConfig() const {
        return m_cnfg;
    }

    ISocketListener *getListener() {
        return m_listener;
    }

    const std::chrono::milliseconds &getIdleTimeout() const {
        return m_idleTimeout;
    }

private:
    /**
     * Socket configuration
     */
    SocketConfig m_cnfg;

    /** Socket listener which wants to receive event new
     * connection
     */
    ISocketListener *m_listener;

    /**
     * Sleep time in idle state
     */
    std::chrono::milliseconds m_idleTimeout;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_SOCKETMANAGERSTATEMACHINE_H
