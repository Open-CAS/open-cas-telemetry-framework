/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_TRACEMANAGER_H
#define SOURCE_OCTF_INTERFACE_TRACEMANAGER_H

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <octf/interface/IRingTraceProducer.h>
#include <octf/interface/ITraceExecutor.h>
#include <octf/interface/ITraceSerializer.h>
#include <octf/node/INode.h>
#include <octf/node/NodeId.h>
#include <octf/proto/InterfaceTraceCreating.pb.h>

namespace octf {

static constexpr char TRACE_FILE_PREFIX[] = "octf.trace.";
static constexpr char SUMMARY_FILE_NAME[] = "octf.summary";

class TraceJob;

enum class TracingState {
    UNDEFINED = 0,
    NOT_STARTED,
    INITIALIZING,
    RUNNING,
    COMPLETE,
    ERROR
};

/**
 * @brief Class for management of multiple jobs each of which is collecting
 * traces
 *
 * Class receives a callback and forwards it to appropriate job;
 */
class TraceManager {
public:
    TraceManager(const NodePath &ownerNodePath, ITraceExecutor *executor);
    ~TraceManager();

    /**
     * @brief Starts jobs which run consuming IO traces and serializing them
     *
     * Spawns a management thread which will execute jobs (number provided by
     * executor), each with the given maximum time duration and a memory pool
     * (which will be divided between each job)
     *
     * @note Once all setup ITraceExecutor::startTrace is called
     *
     * @param maxDuration Max trace duration time (in seconds)
     * @param maxFileSizeInMiB Max size of trace file (in MiB)
     * @param circBufferSizeInMiB Size of the internal trace buffer (in MiB)
     * @param label User defined label
     * @param serializerType Serializer Type
     */
    void startJobs(uint32_t maxDuration,
                   uint64_t maxFileSizeInMiB,
                   uint32_t circBufferSizeInMiB,
                   const std::string &label,
                   SerializerType serializerType);
    void stopJobs();
    /**
     * @brief Gets the sum of trace files' sizes (in MiB) from all child jobs
     */
    int64_t getTraceSizeMiB() const;
    /**
     * @brief Gets the total amount of trace events from all child jobs
     */
    int64_t getTraceCount() const;
    /**
     * @brief Gets the sum of dropped events traces from all child jobs
     */
    int64_t getDroppedTraceCount() const;
    /**
     * @brief Gets the duration of the last/current trace collection
     */
    int64_t getDuration(TracingState state) const;
    /**
     * @brief Gets date and time of starting trace, format: dd-mm-yyyy hh:mm
     * If tracing hasn't started yet, returns empty string.
     *
     * @param state Current state of tracing
     */
    std::string getTraceStartDateTime(TracingState state) const;
    /**
     * @brief Gets numer of IO queues
     */
    int64_t getQueueCount() const;
    /**
     * @brief Gets user defined label
     *
     * @return User label
     */
    const std::string &getLabel() const;
    /**
     * @brief Gets the overall state of the last/current trace.
     */
    TracingState getState();
    /**
     * @brief Checks state of jobs and updates general tracing state
     * accordingly. Calling this function causes recalculation and update of
     * the tracing state
     * @note This method should not be called anywhere in setState() method
     * as it also calls setState()
     */
    void updateState();
    /**
     * @brief Sets specified state and updates trace summary file.
     * @param state State to be set
     */
    void setState(TracingState state);
    /**
     * @brief Fills summary with current values
     * @param summary Summary to be filled
     * @param state Current state of tracing
     */
    void fillTraceSummary(proto::TraceSummary *summary,
                          TracingState state) const;
    /**
     * @brief Pushes a trace to the given job (if found)
     *
     * @retval 0 Trace pushed successfully
     * @retval -EINVAL Trace is invalid
     * @retval -EPERM Invalid open mode, only producer can push events
     * @retval -ENOSPC No space in memory pool to store event
     */
    int pushTrace(uint32_t jobIndex,
                  const void *trace,
                  const uint32_t traceSize);

private:
    /**
     * @brief Creates a management thread which will spawn the appropriate
     * number of child jobs and manage their resources
     */
    void handleJobs();
    void joinThread();
    /**
     * @brief Creates and starts appropriate number of jobs
     */
    void setupJobs();
    /**
     * @brief Clears the job container
     */
    void deleteJobs();

    /**
     * @brief Retrieves parsed protobuf trace version
     */
    int32_t getTraceVersion() const;
    /**
     * @brief Creates trace files directory
     */
    void initializeTraceDirectory();
    /**
     * @brief Gets the sum of trace files' sizes (in bytes) from all child jobs
     */
    int64_t getTraceSize() const;
    /**
     * @brief Node path to owner node
     */
    const NodePath m_ownerNodePath;
    /**
     * @brief Module implementing start/stop trace functions
     */
    ITraceExecutor *m_executor;
    /**
     * @brief Thread for managing child jobs
     */
    std::thread m_thread;
    /**
     * @brief Stop flag for m_thread
     */
    std::atomic<bool> m_finish;
    /**
     * @brief Current state of the trace collection
     */
    std::atomic<TracingState> m_state;
    /**
     * @brief Mutex for synchronous start/stop trace calls
     */
    std::mutex m_traceManagementMutex;
    std::vector<std::unique_ptr<TraceJob>> m_jobs;
    /**
     * @brief Maximum time duration (in seconds) during which a given job will
     * execute
     */
    uint32_t m_maxDuration;
    /**
     * @brief Starting moment of trace collection
     */
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    /**
     * @brief Ending moment of trace collection
     */
    std::chrono::time_point<std::chrono::steady_clock> m_endTime;
    /**
     * @brief Maximum (total sum from all jobs) trace file size (in bytes) in
     * which traces will be saved, after reaching this value all jobs will stop
     * executing
     */
    int64_t m_maxFileSize;
    /**
     * @brief How many jobs will be spawned when startJobs is called
     */
    uint32_t m_numberOfJobs;
    /**
     * @brief Memory pool size in MiB, divided between child jobs for their
     * circular buffers
     */
    uint32_t m_memoryPoolSizeMiB;
    /**
     * @brief Describes the type of output for traces (e.g. file,
     * database)
     */
    SerializerType m_serializerType;
    /**
     * @brief The root output directory for traces for a given plugin instance's
     * trace collection as a path relative to the general trace directory.
     */
    std::string m_traceDirRelativePath;
    /**
     * @brief User defined label
     */
    std::string m_label;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_TRACEMANAGER_H
