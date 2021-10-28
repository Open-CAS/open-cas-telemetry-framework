/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERNAL_TRACEJOB_H
#define SOURCE_OCTF_INTERFACE_INTERNAL_TRACEJOB_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>

#include <octf/interface/IRingTraceProducer.h>
#include <octf/interface/ITraceConverter.h>
#include <octf/interface/ITraceSerializer.h>
#include <octf/interface/TraceManager.h>
#include <octf/proto/InterfaceTraceCreating.pb.h>
#include <octf/trace/trace.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Class for management of traces from an outside source within a single
 * running thread
 *
 * Class receives a callback, puts traces into a circular buffer and serializes
 * them when ready
 */
class TraceJob : public NonCopyable {
public:
    TraceJob(ITraceExecutor *executor,
             uint32_t maxDuration,
             uint32_t queueId,
             const std::string &outputFileName,
             uint32_t memoryPoolSize,
             SerializerType serializerType);
    virtual ~TraceJob();

    /**
     * @brief Returns the written file size
     */
    int64_t getTraceSize() const;

    /**
     * @brief Returns the amount of received traces
     */
    int64_t getTraceCount() const;

    /**
     * @brief Returns the amount of unwritten traces (due to
     * running out of space in circular buffer)
     */
    int64_t getDroppedTraceCount() const;

    int32_t getTraceVersion() const;

    TracingState getState() const;
    /**
     * @brief Waits for the execution of the job
     */
    void joinThread();
    /**
     * @brief Starts the execution of the job
     */
    void startJobThread();
    /**
     * @brief Stops the execution of the job
     */
    void stopJobThread();
    /**
     * @brief Waits until job is ready to receive traces
     */
    void waitForJobInitialization();
    /**
     * @brief Pushes a trace to the given producer (if found)
     *
     * @retval 0 Trace pushed successfully
     * @retval -EINVAL Trace is invalid
     * @retval -EPERM Invalid open mode, only producer can push events
     * @retval -ENOSPC No space in memory pool to store event
     */
    int pushTrace(const void *trace, const uint32_t traceSize);

private:
    /**
     * @brief Executes trace collecting and handles the
     * internal state based on results (endTime, TracingStatus)
     */
    void doWork();
    /**
     * @brief Serialize trace data
     */
    void serialize(const void *data, uint32_t size);
    /**
     * @brief Handles
     * consuming traces from the circular buffer into output file
     */
    void consumeTraces();

    std::thread m_thread;
    /**
     * @brief Starting moment of the job
     */
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    /**
     * @brief Variable triggered when the job is ready for receiving traces
     */
    std::condition_variable m_initializationFinished;
    /**
     * @brief Mutex for setting state changes and simultaneous synchronous
     * access via m_initializationFinished. It's needed, so the
     * condition_variable.wait() function's Predicate won't have a race
     * condition
     */
    std::mutex m_stateMutex;
    /**
     * @brief Current state (started/running/error/finished) of the job
     */
    std::atomic<TracingState> m_state;
    /**
     * @brief Maximum time duration (in seconds) of job's execution
     */
    uint32_t m_maxDuration;
    /**
     * @brief Handle for the circular buffer reader
     */
    octf_trace_t m_traceConsumerHandle;
    /**
     * @brief Total amount of trace events
     */
    std::atomic<int64_t> m_traceCount;
    /**
     * @brief Variable is set if the job is currently parsing traces from
     * circular buffer
     */
    std::atomic<bool> m_processingTraces;
    /**
     * @brief Module that handles writing traces to a file
     */
    std::unique_ptr<ITraceSerializer> m_serializer;
    /**
     * @brief Module that handles parsing traces
     */
    std::unique_ptr<ITraceConverter> m_converter;
    /**
     * @brief Module that handles translating trace events to a file-writable
     * format
     */
    ITraceExecutor *m_executor;
    /**
     * @brief Trace producer
     */
    std::unique_ptr<IRingTraceProducer> m_producer;
};
}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERNAL_TRACEJOB_H
