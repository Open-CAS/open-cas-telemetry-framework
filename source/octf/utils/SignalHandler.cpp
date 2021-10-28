/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/SignalHandler.h>

#include <algorithm>
#include <csignal>
#include <octf/utils/Exception.h>

namespace octf {

SignalHandler::SignalHandler()
        : NonCopyable()
        , m_signalList()
        , m_sigset()
        , m_wait()
        , m_error(0) {
    sigemptyset(&m_sigset);
    sem_init(&m_wait, 0, 0);
}

SignalHandler::~SignalHandler() {
    try {
        clearAllSignals();
    } catch (Exception &) {
    }
}

SignalHandler &SignalHandler::get() {
    static SignalHandler handler;
    return handler;
}

void SignalHandler::registerSignal(int sig) {
    if (std::find(m_signalList.begin(), m_signalList.end(), sig) ==
        m_signalList.end()) {
        if (SIG_ERR == std::signal(sig, SignalHandler::onSignal)) {
            throw Exception("Cannot register signal handler");
        }

        m_signalList.push_back(sig);
    }
}

void SignalHandler::unregisterSignal(int sig) {
    auto iter = std::find(m_signalList.begin(), m_signalList.end(), sig);

    if (iter != m_signalList.end()) {
        if (SIG_ERR == std::signal(sig, SIG_DFL)) {
            // According to documentation SIG_ERR indicates an error
            throw Exception("Cannot unregister signal handler");
        }

        m_signalList.erase(iter);
    }
}

void SignalHandler::clearAllSignals() {
    bool failure = false;

    for (auto iter = m_signalList.begin(); iter != m_signalList.end();) {
        if (SIG_ERR == std::signal(*iter, SIG_DFL)) {
            // According to documentation SIG_ERR indicates an error
            failure = true;

            iter = m_signalList.erase(iter);

        } else {
            iter++;
        }
    }

    if (failure) {
        throw Exception("Cannot unregister signal handler");
    }
}

int SignalHandler::wait() {
    int signalReceived = 0;

    do {
        if (m_error) {
            throw Exception("Error indicated by signal handler");
        }

        if (sem_wait(&m_wait)) {
            throw Exception("Error while waiting on signal");
        }

        for (int sig : m_signalList) {
            // Check in signal set if given signal is raised
            if (sigismember(&m_sigset, sig)) {
                signalReceived = sig;

                // signal received, clear it from set
                if (sigdelset(&m_sigset, sig)) {
                    throw Exception("Cannot clear signals set");
                }

                break;
            }
        }

    } while (0 == signalReceived);

    return signalReceived;
}

void SignalHandler::onSignal(int sig) {
    SignalHandler &handler = get();

    // Keep information in signal set which signals have been received.
    // Program's thread will scan signal set to find out which ones were
    // received.
    if (sigaddset(&handler.m_sigset, sig)) {
        handler.m_error = 1;
    }

    if (sem_post(&handler.m_wait)) {
        handler.m_error = 1;
    }
}

void SignalHandler::sendSignal(int sig) {
    onSignal(sig);
}

}  // namespace octf
