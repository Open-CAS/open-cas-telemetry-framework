/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/Semaphore.h>
#include <atomic>

namespace octf {

Semaphore::Semaphore(unsigned int value)
        : m_count(value)
        , m_lock()
        , m_cv() {}

Semaphore::~Semaphore() {}

void Semaphore::post() {
    std::unique_lock<std::mutex> lock(m_lock);
    m_count++;
    m_cv.notify_one();
}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(m_lock);

    while (!m_count) {
        m_cv.wait(lock);
    }

    m_count--;
}

bool Semaphore::waitFor(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(m_lock);

    while (!m_count) {
        if (m_cv.wait_for(lock, timeout) == std::cv_status::timeout) {
            return false;
        }
    }

    m_count--;

    return true;
}

}  // namespace octf
