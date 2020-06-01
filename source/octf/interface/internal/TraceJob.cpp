/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/internal/TraceJob.h>

#include <mutex>
#include <string>

#include <octf/interface/internal/FileTraceSerializer.h>
#include <octf/trace/iotrace_event.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/SizeConversion.h>

namespace octf {

TraceJob::TraceJob(ITraceExecutor *executor,
                   uint32_t maxDuration,
                   uint32_t queueId,
                   const std::string &outputFileName,
                   uint32_t memoryPoolSize,
                   SerializerType serializerType)
        : NonCopyable()
        , m_thread()
        , m_state(TracingState::NOT_STARTED)
        , m_maxDuration(maxDuration)
        , m_traceConsumerHandle(0)
        , m_traceCount(0)
        , m_processingTraces(false)
        , m_executor(executor)
        , m_producer(executor->createProducer(queueId)) {
    m_producer->initRing(memoryPoolSize);

    int result = octf_trace_open(
            static_cast<void *>(m_producer->getBuffer()), m_producer->getSize(),
            m_producer->getConsumerHeader(), octf_trace_open_mode_consumer,
            &m_traceConsumerHandle);

    if (result) {
        m_producer->deinitRing();
        throw Exception("Failed to initialize trace buffer for consumer.");
    }

    m_converter = m_executor->createTraceConverter();

    switch (serializerType) {
    case SerializerType::FileSerializer:
        m_serializer = std::unique_ptr<FileTraceSerializer>(
                new FileTraceSerializer(outputFileName));
        break;
    default:
        octf_trace_close(&m_traceConsumerHandle);
        m_traceConsumerHandle = nullptr;
        m_producer->deinitRing();
        throw Exception("Unknown trace serializer type.");
    }

    if (!m_serializer->open()) {
        octf_trace_close(&m_traceConsumerHandle);
        m_traceConsumerHandle = nullptr;
        m_producer->deinitRing();
        throw Exception("Cannot open file '" + outputFileName + "'");
    }
}

TraceJob::~TraceJob() {
    stopJobThread();
    joinThread();
    if (m_traceConsumerHandle) {
        octf_trace_close(&m_traceConsumerHandle);
    }
    m_producer->deinitRing();
}

int64_t TraceJob::getTraceSize() const {
    return m_serializer->getDataSize();
}

int64_t TraceJob::getTraceCount() const {
    return m_traceCount.load();
}

int64_t TraceJob::getDroppedTraceCount() const {
    return octf_trace_get_lost_count(m_traceConsumerHandle);
}

TracingState TraceJob::getState() const {
    return m_state.load();
}

int32_t TraceJob::getTraceVersion() const {
    return m_converter->getTraceVersion();
}

void TraceJob::startJobThread() {
    if (!m_thread.joinable()) {
        m_thread = std::thread(&TraceJob::doWork, this);

        int cpuAffinity = m_producer->getCpuAffinity();
        if (cpuAffinity != NO_CPU_AFFINITY) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpuAffinity, &cpuset);
            int result = pthread_setaffinity_np(m_thread.native_handle(),
                                                sizeof(cpuset), &cpuset);
            if (result) {
                throw Exception("Failed to set CPU affinity for job thread");
            }
        }
    }
}

void TraceJob::stopJobThread() {
    m_producer->stop();
}

void TraceJob::waitForJobInitialization() {
    std::unique_lock<std::mutex> lock(m_stateMutex);
    m_initializationFinished.wait(
            lock, [this] { return m_state != TracingState::NOT_STARTED; });
}

void TraceJob::doWork() {
    TracingState endState;
    m_startTime = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_state = TracingState::RUNNING;
        m_initializationFinished.notify_all();
    }
    log::verbose << "Trace collecting has started of queue"
                 << m_producer->getQueueId() << std::endl;

    try {
        consumeTraces();
        endState = TracingState::COMPLETE;

    } catch (Exception &e) {
        endState = TracingState::ERROR;
        log::cerr << e.getMessage() << std::endl;
    } catch (std::exception &e) {
        endState = TracingState::ERROR;
        log::cerr << e.what() << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_state = endState;
    }
    m_serializer->close();
    log::verbose << "Trace collecting has completed of queue "
                 << m_producer->getQueueId() << std::endl;
}

int TraceJob::pushTrace(const void *trace, const uint32_t traceSize) {
    return m_producer->pushTrace(trace, traceSize);
}

void TraceJob::serialize(const void *data, uint32_t size) {
    bool serialized = false;
    if (m_converter) {
        auto protoBuffer = m_converter->convertTrace(data, size);

        if (protoBuffer) {
            serialized = m_serializer->serialize(protoBuffer);
        }
    } else {
        serialized = m_serializer->serialize(data, size);
    }

    if (serialized) {
        m_traceCount++;
    } else {
        throw Exception("Failed to serialize trace data");
    }
}

void TraceJob::consumeTraces() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    auto endTime = m_startTime + std::chrono::seconds(m_maxDuration);
    bool finish = false;

    // The loop executes until the desired time passes or someone forces it to
    // end
    do {
        m_processingTraces = false;
        if (!m_producer->wait(endTime)) {
            finish = true;
        }
        m_processingTraces = true;
        // If we're woken up (via timeout or outside trigger), try to empty the
        // circular buffer
        while (!octf_trace_is_empty(m_traceConsumerHandle)) {
            octf_trace_event_handle_t eventHandle = {};
            bool circBufferNotAvailable = false;
            void *traceBuffer = NULL;
            uint32_t traceSize = 0;

            int result = octf_trace_get_rd_buffer(m_traceConsumerHandle,
                                                  &eventHandle, &traceBuffer,
                                                  &traceSize);
            switch (result) {
            case 0:
                this->serialize(traceBuffer, traceSize);
                result = octf_trace_release_rd_buffer(m_traceConsumerHandle,
                                                      eventHandle);
                if (result) {
                    throw Exception("Failed to release trace data, error: " +
                                    std::to_string(result));
                }
                break;
            case -EAGAIN:
            case -EBUSY:
                circBufferNotAvailable = true;
                break;
            case -EINVAL:
            case -EPERM:
            case -EBADF:
            case -ENOSPC:
            default:
                throw Exception("Failed to retrieve trace data, error: " +
                                std::to_string(result));
            }
            if (circBufferNotAvailable) {
                // We want to break (instead of continue in the switch), so that
                // we won't be potentially stuck in the inner loop and can have
                // tracing stopped by outside forces/timeout
                break;
            }
        }
    } while (!finish);
}

void TraceJob::joinThread() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

}  // namespace octf
