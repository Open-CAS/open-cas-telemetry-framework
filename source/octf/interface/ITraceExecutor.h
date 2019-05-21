/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACEEXECUTOR_H
#define SOURCE_OCTF_INTERFACE_ITRACEEXECUTOR_H

#include <cstdint>
#include <octf/interface/ITraceConverter.h>

namespace octf {

class IRingTraceProducer;

/**
 * @brief The implementation of this interface should be able to notify the
 * traced module of starting and stopping of trace collecting
 */
class ITraceExecutor {
public:
    virtual ~ITraceExecutor() = default;
    /**
     * @brief A specific implementation of the executor will notify the traced
     * module (eg. CAS) to start trace collection. The specific implementation
     * of communication depends on the module.
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool startTrace() = 0;
    /**
     * @brief A specific implementation of the executor will notify the traced
     * module (eg. CAS) to stop trace collection. The specific implementation of
     * communication depends on the module.
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool stopTrace() = 0;
    /**
     * @brief Returns the number of trace queues that should be created.
     */
    virtual uint32_t getTraceQueueCount() = 0;
    /**
     * @brief Creates and returns a pointer to trace producer object.
     * Depending on the implementation, returned objects shall be of appropriate
     * class derived from IRingTraceProducer interface.
     * @return Raw pointer to trace producer object
     */
    virtual std::unique_ptr<IRingTraceProducer> createProducer(
            uint32_t queueId) = 0;
    /**
     * @brief Returns a module capable of parsing traces. If conversion to
     * Google Protocol Buffer is not necessary, returns null
     */
    virtual std::unique_ptr<ITraceConverter> createTraceConverter() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACEEXECUTOR_H
