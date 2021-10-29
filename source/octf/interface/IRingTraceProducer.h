/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_IRINGTRACEPRODUCER_H
#define SOURCE_OCTF_INTERFACE_IRINGTRACEPRODUCER_H

#include <chrono>
#include <cstdint>

#include <octf/trace/trace.h>
namespace octf {

constexpr int NO_CPU_AFFINITY = -1;

/**
 * @brief Abstraction of trace producer utilizing ring (circular buffer)
 * structure.
 */
class IRingTraceProducer {
public:
    virtual ~IRingTraceProducer() = default;

    /**
     * @brief Returns buffer with trace ring (circular buffer)
     */
    virtual char *getBuffer(void) = 0;

    /**
     * @brief Returns size of trace ring (circular buffer)
     */
    virtual size_t getSize(void) const = 0;

    /**
     * @brief Returns buffer with trace consumer header
     */
    virtual octf_trace_hdr_t *getConsumerHeader(void) = 0;

    /**
     * @brief Waits until new traces are available or this->stop() is called or
     *         until specified end time.
     * @retval true if new events are available and tracing is not stopped or
     * timed out
     * @retval false if timed out or stopped
     */
    virtual bool wait(
            std::chrono::time_point<std::chrono::steady_clock> &endTime) = 0;

    /**
     * @brief Stops producing traces and signal job waiting in this->wait()
     */
    virtual void stop(void) = 0;

    /**
     * @brief Gets producer CPU affinity.
     *
     * @retval -1 no affinity
     * @retval >= 0 CPU affinity
     */
    virtual int getCpuAffinity(void) = 0;

    /**
     * @brief Initializes ring (circular buffer) of given size
     */
    virtual void initRing(uint32_t memoryPoolSize) = 0;

    /**
     * @brief Deinitializes ring (circular buffer) of given size
     */
    virtual void deinitRing() = 0;

    /**
     * @brief Pushes a given trace to the ring (circular buffer)
     *
     * @retval 0 Trace pushed successfully
     * @retval -EINVAL Trace is invalid
     * @retval -EPERM Invalid open mode, only producer can push events
     * @retval -ENOSPC No space in memory pool to store event
     */
    virtual int pushTrace(const void *trace, const uint32_t traceSize) = 0;

    /**
     * @brief Gets the queue id of this producer
     */
    virtual int32_t getQueueId() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_IRINGTRACEPRODUCER_H
