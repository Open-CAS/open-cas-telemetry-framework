/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_TRACEPRODUCERLOCAL_H
#define SOURCE_OCTF_INTERFACE_TRACEPRODUCERLOCAL_H

#include <atomic>
#include <condition_variable>
#include <vector>
#include <octf/interface/IRingTraceProducer.h>
#include <octf/trace/trace.h>

namespace octf {

/**
 * @brief Trace producer implementation for the case where trace
 * buffer producer runs in the same process as trace consumer.
 */
class TraceProducerLocal : public IRingTraceProducer {
public:
    TraceProducerLocal(uint32_t queueId);
    virtual ~TraceProducerLocal();

    char *getBuffer(void) override;

    size_t getSize(void) const override;

    octf_trace_hdr_t *getConsumerHeader(void) override;

    bool wait(std::chrono::time_point<std::chrono::steady_clock> &endTime)
            override;

    void stop(void) override;

    void initRing(uint32_t memoryPoolSize) override;

    void deinitRing() override;

    int pushTrace(const void *trace, const uint32_t traceSize) override;

    int getCpuAffinity(void) override;

    int32_t getQueueId() override;

protected:
    octf_trace_t getTraceProducerHandle() const {
        return m_traceProducerHandle;
    }

private:
    /**
     * @brief Handle for the producer's trace
     */
    octf_trace_t m_traceProducerHandle;

    /**
     * @brief Memory pool used for ring (circular buffer) and consumer header
     */
    std::vector<char> m_ringMemoryPool;

    /**
     * @brief Set to true when this->stop is called
     */
    bool m_stop;

    /**
     * @brief Notified when thread waiting on traces should be signalled
     */
    std::condition_variable m_checkTrigger;

    /**
     * @brief When ring free size is below this value,
     * user waiting in this->wait will be woken up
     */
    uint32_t m_ringTriggerSize;

    /**
     * @brief Mutex for @m_checkTrigger stop condition
     */
    std::mutex m_mutex;

    /**
     * @brief Queue id for this producer
     */
    uint32_t m_queueId;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_TRACEPRODUCERLOCAL_H
