/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/socket/SocketManager.h>

#include <thread>
#include <octf/socket/ISocketListener.h>
#include <octf/socket/internal/SocketManagerStateMachine.h>
#include <octf/socket/internal/SocketManagerStateMachineClient.h>
#include <octf/socket/internal/SocketManagerStateMachineServer.h>
#include <octf/utils/Exception.h>

namespace octf {

SocketManager::SocketManager(ISocketListener *listener,
                             const SocketConfig &cnfg)
        : m_sm()
        , m_smThread()
        , m_active(false) {
    switch (cnfg.type) {
    case SocketType::Server:
        m_sm = std::unique_ptr<SocketManagerStateMachineServer>(
                new SocketManagerStateMachineServer(listener, cnfg));
        break;

    case SocketType::Client:
        m_sm = std::unique_ptr<SocketManagerStateMachineClient>(
                new SocketManagerStateMachineClient(listener, cnfg));
        break;

    default:
        throw Exception("Unknown socket type.");
        break;
    }
}

SocketManager::~SocketManager() {}

bool SocketManager::isActive() const {
    return m_active;
}

void SocketManager::activate() {
    if (m_smThread.joinable() || m_active) {
        // Already started, do nothing
        return;
    }

    m_active = true;

    m_smThread = std::thread([this]() { run(); });
}

void SocketManager::deactivate() {
    if (!isActive()) {
        // Already stopped, do nothing
        return;
    }

    m_active = false;

    m_sm->close();

    if (m_smThread.joinable()) {
        m_smThread.join();
    }
}

void SocketManager::run() {
    SocketManagerStateMachine::State state =
            SocketManagerStateMachine::State::Initializing;

    while (m_active) {
        switch (state) {
        case SocketManagerStateMachine::State::Idle:
            state = m_sm->idle();
            break;

        case SocketManagerStateMachine::State::Initializing:
            state = m_sm->init();
            break;

        case SocketManagerStateMachine::State::Working:
            state = m_sm->work();
            break;

        default:
            break;
        }
    }
}

const SocketConfig &SocketManager::getSocketConfig() const {
    return m_sm->getSocketConfig();
}

}  // namespace octf
