/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_SEMAPHORE_H
#define SOURCE_OCTF_UTILS_SEMAPHORE_H

#include <condition_variable>
#include <mutex>

namespace octf {

/**
 * @brief Wrapper for semaphore
 */
class Semaphore {
public:
    /**
     * @param value Starting value semaphore
     */
    Semaphore(unsigned int value = 0);
    virtual ~Semaphore();

    /**
     * @brief Posts semaphore (increments semaphores values)
     */
    void post();

    /**
     * @brief Waits for semaphore until semaphore is posted
     */
    void wait();

    /**
     * @brief Waits for semaphore until semaphore is posted or timeout occurred
     *
     * @param timeout Timeout of semaphore posting
     *
     * @retval true semaphore posted and no timeout
     * @retval false timeout occurred
     */
    bool waitFor(std::chrono::milliseconds timeout);

private:
    int32_t m_count;
    std::mutex m_lock;
    std::condition_variable m_cv;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_SEMAPHORE_H
