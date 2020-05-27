/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/TraceManager.h>

#include <time.h>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <octf/interface/internal/TraceJob.h>
#include <octf/node/NodeId.h>
#include <octf/utils/DateTime.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtoConverter.h>
#include <octf/utils/ProtobufReaderWriter.h>
#include <octf/utils/SizeConversion.h>

namespace octf {

TraceManager::TraceManager(const NodePath &ownerNodePath,
                           ITraceExecutor *executor)
        : m_ownerNodePath(ownerNodePath)
        , m_executor(executor)
        , m_thread()
        , m_finish(false)
        , m_state(TracingState::NOT_STARTED)
        , m_traceManagementMutex()
        , m_jobs()
        , m_maxDuration(0)
        , m_maxFileSize(0)
        , m_numberOfJobs(executor->getTraceQueueCount())
        , m_memoryPoolSizeMiB(0)
        , m_serializerType(SerializerType::FileSerializer)
        , m_traceDirRelativePath("") {}

TraceManager::~TraceManager() {
    m_finish = true;
    joinThread();
    deleteJobs();
}

void TraceManager::joinThread() {
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void TraceManager::handleJobs() {
    m_endTime = m_startTime = std::chrono::steady_clock::now();
    auto endTime = m_startTime + std::chrono::seconds(m_maxDuration);
    try {
        setupJobs();

        if (!m_executor->startTrace()) {
            throw Exception("Error sending start trace request");
        }

        while (!m_finish) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            // TODO (kozlowsk) Should maxfilesize be checked here every x
            // seconds or per job?
            if (std::chrono::steady_clock::now() > endTime ||
                getTraceSize() >= m_maxFileSize) {
                break;
            }
        }

        if (!m_executor->stopTrace()) {
            throw Exception("Error sending stop trace request");
        }
    } catch (Exception &e) {
        log::cerr << e.getMessage() << std::endl;
        setState(TracingState::ERROR);
    } catch (std::exception &e) {
        log::cerr << e.what() << std::endl;
        setState(TracingState::ERROR);
    }

    for (const auto &job : m_jobs) {
        job->stopJobThread();
    }
    for (const auto &job : m_jobs) {
        job->joinThread();
    }

    log::cout << "Trace collecting has completed" << std::endl;
    updateState();
}

void TraceManager::initializeTraceDirectory() {
    std::string directoryPathRelative;
    std::string directoryPathAbsolute;
    std::string dateTime = datetime::getFormattedDateTime(
            std::chrono::steady_clock::now(), "%Y-%m-%d_%H:%M:%S");

    using namespace fsutils;

    const auto &traceDir = getFrameworkConfiguration().getTraceDir();
    if (!checkPermissions(traceDir, PermissionType::Execute) ||
        !checkPermissions(traceDir, PermissionType::ReadWrite)) {
        throw Exception("No access to trace directory");
    }

    directoryPathRelative =
            getFrameworkConfiguration().getNodePathBasename(m_ownerNodePath) +
            "/" + dateTime;

    directoryPathAbsolute = traceDir + "/" + directoryPathRelative;
    if (!createDirectory(directoryPathAbsolute)) {
        throw Exception("Error creating trace directory " +
                        directoryPathAbsolute);
    }

    m_traceDirRelativePath = directoryPathRelative;
}

void TraceManager::setupJobs() {
    // Split the common circular buffer so each job gets a share of it
    std::size_t jobBufferSize =
            MiBToBytes(m_memoryPoolSizeMiB) / m_numberOfJobs;

    // TODO(ajrutkow): change ring buffer size to 64 bit across source code
    if (jobBufferSize > std::numeric_limits<uint32_t>::max() ||
        jobBufferSize == 0) {
        throw Exception("Incorrect circular buffer size");
    }

    for (uint32_t i = 0; i < m_numberOfJobs; i++) {
        // If one of the constructors throws an exception, the others will
        // be stopped and cleaned up at the end of handleJobs function
        const auto jobFileName = getFrameworkConfiguration().getTraceDir() +
                                 "/" + m_traceDirRelativePath + "/" +
                                 TRACE_FILE_PREFIX + std::to_string(i);
        auto job = std::unique_ptr<TraceJob>(new TraceJob(
                m_executor, m_maxDuration, i, jobFileName,
                static_cast<uint32_t>(jobBufferSize), m_serializerType));
        job->startJobThread();
        m_jobs.push_back(std::move(job));
    }

    // Wait until each job is ready for receiving traces before leaving this
    // function
    for (const auto &job : m_jobs) {
        job->waitForJobInitialization();
    }

    setState(TracingState::RUNNING);

    log::cout << "Trace collecting has started" << std::endl;
}

void TraceManager::startJobs(uint32_t maxDuration,
                             uint64_t maxFileSizeInMiB,
                             uint32_t circBufferSizeInMiB,
                             const std::string &label,
                             SerializerType serializerType) {
    std::lock_guard<std::mutex> lock(m_traceManagementMutex);
    auto state = getState();

    // If there are still executing jobs, don't do anything
    if (state != TracingState::RUNNING && state != TracingState::INITIALIZING) {
        if (m_serializerType == SerializerType::FileSerializer) {
            initializeTraceDirectory();
        }

        // We're joining the thread in case it has timed out or ran out of file
        // space; the total status might technically be not running, but the
        // thread might still be winding down
        joinThread();

        deleteJobs();
        m_finish = false;
        // Setting initializing state in here, so we won't accidentally
        // spawn a second management thread in case a second startJobs call
        // would come before handleJobs thread would actually start execution
        setState(TracingState::INITIALIZING);

        m_maxDuration = maxDuration;
        m_maxFileSize = MiBToBytes(maxFileSizeInMiB);
        m_memoryPoolSizeMiB = circBufferSizeInMiB;
        m_serializerType = serializerType;
        m_label = label;

        m_thread = std::thread(&TraceManager::handleJobs, this);
    } else {
        throw Exception("Tracing already started.");
    }
}

void TraceManager::stopJobs() {
    std::lock_guard<std::mutex> lock(m_traceManagementMutex);
    m_finish = true;
    joinThread();

    // Update state of tracing
    updateState();
}

void TraceManager::deleteJobs() {
    m_jobs.clear();
}

void TraceManager::fillTraceSummary(proto::TraceSummary *summary,
                                    TracingState state) const {
    protoconverter::convertNodePath(summary->mutable_sourcenode(),
                                    m_ownerNodePath);
    summary->set_tracepath(m_traceDirRelativePath);
    summary->set_tracestartdatetime(getTraceStartDateTime(state));
    summary->set_traceduration(getDuration(state));
    summary->set_tracesize(getTraceSizeMiB());
    summary->set_tracedevents(getTraceCount());
    summary->set_droppedevents(getDroppedTraceCount());
    summary->set_queuecount(getQueueCount());
    summary->set_label(getLabel());
    summary->set_version(getTraceVersion());

    proto::TraceState tracingState = proto::TraceState::UNDEFINED;
    switch (state) {
    case TracingState::UNDEFINED:
        tracingState = proto::TraceState::UNDEFINED;
        break;
    case TracingState::NOT_STARTED:
        tracingState = proto::TraceState::NOT_STARTED;
        break;
    case TracingState::INITIALIZING:
        tracingState = proto::TraceState::INITIALIZING;
        break;
    case TracingState::RUNNING:
        tracingState = proto::TraceState::RUNNING;
        break;
    case TracingState::COMPLETE:
        tracingState = proto::TraceState::COMPLETE;
        break;
    case TracingState::ERROR:
        tracingState = proto::TraceState::ERROR;
        break;
    default:
        throw Exception("Tracing State error.");
        break;
    }
    summary->set_state(tracingState);
}

std::string TraceManager::getTraceStartDateTime(TracingState state) const {
    if (state == TracingState::NOT_STARTED) {
        return "";
    }

    return datetime::getFormattedDateTime(m_startTime);
}

int64_t TraceManager::getQueueCount() const {
    return m_jobs.size();
}

int64_t TraceManager::getTraceSize() const {
    int64_t result = 0;
    for (const auto &job : m_jobs) {
        result += job->getTraceSize();
    }

    return result;
}

int32_t TraceManager::getTraceVersion() const {
    int32_t version = 0;

    if (m_jobs.size()) {
        version = m_jobs[0]->getTraceVersion();
    }

    return version;
}

int64_t TraceManager::getTraceSizeMiB() const {
    // Return the size in MiB (rounding up)
    int64_t result = BytesToMiBCeiling(getTraceSize());
    return result;
}

int64_t TraceManager::getTraceCount() const {
    int64_t result = 0;
    for (const auto &job : m_jobs) {
        result += job->getTraceCount();
    }

    return result;
}

int64_t TraceManager::getDroppedTraceCount() const {
    int64_t result = 0;
    for (const auto &job : m_jobs) {
        result += job->getDroppedTraceCount();
    }

    return result;
}

int64_t TraceManager::getDuration(TracingState state) const {
    using std::chrono::duration_cast;
    using std::chrono::seconds;
    using std::chrono::steady_clock;
    using std::chrono::time_point;

    if (state == TracingState::NOT_STARTED ||
        state == TracingState::INITIALIZING) {
        return 0;
    }

    time_point<steady_clock> endTime;
    if (state == TracingState::RUNNING) {
        endTime = steady_clock::now();
    } else {
        endTime = m_endTime;
    }

    return duration_cast<seconds>(endTime - m_startTime).count();
}

const std::string &TraceManager::getLabel() const {
    return m_label;
}

TracingState TraceManager::getState() {
    updateState();
    return m_state.load();
}

void TraceManager::updateState() {
    // If we're currently in running state (i.e. jobs have been successfully
    // spawned), ask for their status and update as required
    if (m_state == TracingState::RUNNING) {
        // Assume no tracing was started
        TracingState state = TracingState::NOT_STARTED;
        bool foundRunningState = false;
        bool foundErrorState = false;
        bool allFinished = true;

        for (const auto &job : m_jobs) {
            switch (job->getState()) {
            case TracingState::NOT_STARTED:
                allFinished = false;
                break;
            case TracingState::RUNNING:
                foundRunningState = true;
                allFinished = false;
                break;
            case TracingState::ERROR:
                foundErrorState = true;
                break;
            default:
                break;
            }
        }

        if (foundRunningState) {
            // If at least one job is still running, return Running state
            state = TracingState::RUNNING;
        } else if (allFinished && foundErrorState) {
            // If all jobs finished running, but at least one had an error
            // return Error state
            state = TracingState::ERROR;
        } else if (allFinished && m_jobs.size() > 0) {
            // If they finished running without an error, the state is Complete
            state = TracingState::COMPLETE;
        }

        setState(state);
    }
}

void TraceManager::setState(TracingState state) {
    if (m_state == TracingState::RUNNING && state != TracingState::RUNNING) {
        m_endTime = std::chrono::steady_clock::now();
    }

    // Update state
    m_state = state;

    if (m_serializerType == SerializerType::FileSerializer) {
        // Update summary  and serialize to file
        proto::TraceSummary summary;
        fillTraceSummary(&summary, state);
        std::string filePath = getFrameworkConfiguration().getTraceDir() + "/" +
                               m_traceDirRelativePath + "/" + SUMMARY_FILE_NAME;
        ProtobufReaderWriter rw(filePath);

        if (!rw.write(summary)) {
            m_state = TracingState::ERROR;
            log::cerr << "Could not write summary file: " << filePath
                      << std::endl;
        }

        // Tracing is finished, make trace summary read only
        if (m_state == TracingState::ERROR ||
            m_state == TracingState::COMPLETE) {
            if (!rw.makeReadOnly()) {
                log::cerr << "Could not make trace summary read only, file "
                          << filePath << std::endl;

                m_state = TracingState::ERROR;
                fillTraceSummary(&summary, TracingState::ERROR);

                if (!rw.write(summary)) {
                    log::cerr << "Could not re-write summary file: " << filePath
                              << std::endl;
                }
            }
        }
    }
}

int TraceManager::pushTrace(uint32_t jobIndex,
                            const void *trace,
                            const uint32_t traceSize) {
    if (jobIndex < m_jobs.size()) {
        return m_jobs[jobIndex]->pushTrace(trace, traceSize);
    }
    return -EINVAL;
}

}  // namespace octf
