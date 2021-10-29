/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXTDELETER_H
#define SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXTDELETER_H
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>

#include <octf/communication/CommunicationManagerServer.h>
#include <octf/communication/internal/ConnectionContext.h>
#include <octf/utils/Semaphore.h>

namespace octf {

/**
 * @brief Utilities class which is responsible for destroying
 *  connection contexts in background thread
 */
class ConnectionContextDeleter {
public:
    ConnectionContextDeleter();

    virtual ~ConnectionContextDeleter();

    /**
     * @brief Adds connection context for deletion
     *
     * @param Inactive connection context to be deleted
     */
    void addConnectionContext(ConnectionContextShRef &&conn);

private:
    /**
     * @brief Flushes all queued connection contexts and destroy them
     */
    void empty();

    /**
     * @brief Gets next connection context for deletion
     *
     * @param[out] connection context to be deleted
     *
     * @retval true indicates that returned objects is valid and can be
     * removed
     * @retval false No more object for destruction
     */
    bool getConnection(ConnectionContextShRef &conn);

private:
    /**
     * Inactive connection list
     */
    std::list<ConnectionContextShRef> m_inactiveConnCntxList;

    /**
     * Synchronization lock for inactive connection contexts list
     */
    std::mutex m_lock;

    /**
     * Semaphore signaling to the thread that there are contexts
     * for deletion
     */
    Semaphore m_sem;

    /**
     * Background thread in context of which contexts are destroyed
     */
    std::thread m_thread;

    /**
     * Indicates that thread shall be run
     */
    std::atomic<bool> m_run;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXTDELETER_H
