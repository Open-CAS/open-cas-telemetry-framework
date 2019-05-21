/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/socket/internal/SocketManagerStateMachine.h>

#include <thread>

namespace octf {

SocketManagerStateMachine::SocketManagerStateMachine(ISocketListener *listener,
                                                     const SocketConfig &cnfg)
        : m_cnfg(cnfg)
        , m_listener(listener)
        , m_idleTimeout(300) {}

SocketManagerStateMachine::~SocketManagerStateMachine() {}

SocketManagerStateMachine::State SocketManagerStateMachine::idle() {
    std::this_thread::sleep_for(getIdleTimeout());
    return State::Initializing;
}

}  // namespace octf
