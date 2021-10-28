/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_SIGNALHANDLER_H
#define SOURCE_OCTF_UTILS_SIGNALHANDLER_H

#include <semaphore.h>
#include <signal.h>
#include <atomic>
#include <list>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Class handling application signals
 */

class SignalHandler final : NonCopyable {
public:
    static SignalHandler &get();

    virtual ~SignalHandler();

    /**
     * @brief Registers signal to handle by application
     *
     * @param sig Signal to register
     */
    void registerSignal(int sig);

    /**
     * @brief Unregisters signal handled by application
     *
     * @param sig Signal to unregister
     */
    void unregisterSignal(int sig);

    /**
     * @brief Unregisters all signals handled by application
     */
    void clearAllSignals();

    /**
     * @brief Waits for a registered signal
     *
     * @return Value of signal which has been received
     */
    int wait();

    /**
     * @brief Sends a signal from the application
     *
     * This allows to send signals programatically from the application
     * to SignalHandler. The signal needs to be registered first
     * to have any effect
     *
     * @param sig Signal to be sent
     */
    void sendSignal(int sig);

private:
    SignalHandler();

    static void onSignal(int sig);

private:
    /**
     * @brief List of registered signals
     *
     */
    std::list<int> m_signalList;

    /**
     * @brief Set of already raised signals
     */
    sigset_t m_sigset;

    /**
     * @brief Semaphore posted from signal handler
     */
    sem_t m_wait;

    /**
     * @brief Variable indicating an error occurred on signal handler
     */
    std::atomic_int m_error;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_SIGNALHANDLER_H
