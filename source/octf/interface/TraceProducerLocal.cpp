/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/interface/TraceProducerLocal.h>

#include <octf/utils/Exception.h>

namespace octf {

static constexpr size_t BUFFER_FREE_SPACE_PERCENTAGE_WAKE_UP_TRIGGER = 75;

TraceProducerLocal::TraceProducerLocal(uint32_t queueId)
        : m_traceProducerHandle(nullptr)
        , m_ringMemoryPool()
        , m_stop(false)
        , m_ringTriggerSize(0)
        , m_queueId(queueId) {}

TraceProducerLocal::~TraceProducerLocal() {
    deinitRing();
}

void TraceProducerLocal::initRing(uint32_t memoryPoolSize) {
    m_ringMemoryPool.resize(memoryPoolSize);
    m_ringTriggerSize =
            memoryPoolSize * BUFFER_FREE_SPACE_PERCENTAGE_WAKE_UP_TRIGGER / 100;

    if (octf_trace_open(m_ringMemoryPool.data(), memoryPoolSize, NULL,
                        octf_trace_open_mode_producer,
                        &m_traceProducerHandle)) {
        throw Exception("Failed to initialize trace producer");
    }

    // reset stop in case circular buffer is re-initialized
    m_stop = false;
}

void TraceProducerLocal::deinitRing() {
    octf_trace_close(&m_traceProducerHandle);
    m_ringMemoryPool.clear();
}

char *TraceProducerLocal::getBuffer() {
    return m_ringMemoryPool.data();
}

size_t TraceProducerLocal::getSize() const {
    return m_ringMemoryPool.size();
}

octf_trace_hdr_t *TraceProducerLocal::getConsumerHeader() {
    return nullptr;
}

void TraceProducerLocal::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_checkTrigger.notify_all();
}

bool TraceProducerLocal::wait(
        std::chrono::time_point<std::chrono::steady_clock> &endTime) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_checkTrigger.wait_until(lock, endTime) == std::cv_status::timeout) {
        return false;
    }

    return !m_stop;
}

int TraceProducerLocal::getCpuAffinity(void) {
    // no thread affinity
    return NO_CPU_AFFINITY;
}

int TraceProducerLocal::pushTrace(const void *trace, const uint32_t traceSize) {
    int result = 0;
    bool stop;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        stop = m_stop;
    }

    if (!stop) {
        if (traceSize > m_ringMemoryPool.size()) {
            result = -ENOSPC;
        } else if (traceSize > 0) {
            result = octf_trace_push(m_traceProducerHandle, trace, traceSize);
            if (!result) {
                // Send the processing trigger if the free space in the circular
                // buffer has reached the threshold
                if (m_ringTriggerSize >
                    octf_trace_get_free_space(m_traceProducerHandle)) {
                    m_checkTrigger.notify_all();
                }
            }
        }
        // trace size of 0 - we don't do anything with it
    } else {
        result = -ENOSPC;
    }

    return result;
}

int32_t TraceProducerLocal::getQueueId() {
    return m_queueId;
}

}  // namespace octf
