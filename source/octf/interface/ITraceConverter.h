/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACECONVERTER_H
#define SOURCE_OCTF_INTERFACE_ITRACECONVERTER_H

#include <google/protobuf/message.h>
#include <cstdint>

namespace octf {

/**
 * @brief The implementation of this interface may convert a
 * trace to Google Protocol Buffers format.
 */
class ITraceConverter {
public:
    virtual ~ITraceConverter() = default;
    /**
     * @brief A specific implementation of the converter may choose to translate
     * the trace event structure to Google Protocol Buffer format (if the trace
     * structure is known), before serializing it. This function is executed in
     * job's context (i.e. not in the context of a pushTrace function), to
     * minimize the impact of trace collection on the traced module's I/O path.
     *
     * @param trace Input trace structure for parsing
     * @param size Input trace structure's size
     *
     * @retval Message - parsed Protocol Buffer structure if successful
     * @retval nullptr - in case of failure
     */
    virtual std::shared_ptr<const google::protobuf::Message> convertTrace(
            const void *trace,
            uint32_t size) = 0;

    /**
     * @brief Returns version of the converted protobuf traces
     *
     * @retval version
     */
    virtual int32_t getTraceVersion() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACECONVERTER_H
