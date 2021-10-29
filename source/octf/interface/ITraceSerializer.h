/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACESERIALIZER_H
#define SOURCE_OCTF_INTERFACE_ITRACESERIALIZER_H

#include <google/protobuf/message.h>

namespace octf {

/**
 * @brief Describes the internal type of trace serialization to be used
 */
enum class SerializerType {
    /**
     * @brief Traces will be serialized to files in directory specified by
     * Storage Analytics configuration file
     */
    FileSerializer = 0
};

/**
 * @brief An implementation of the class should be able to save a Google
 * Protocol Buffer, or an opaque buffer to some output location (e.g. file,
 * database)
 */
class ITraceSerializer {
public:
    virtual ~ITraceSerializer() = default;
    /**
     * @brief Serializes a given buffer to output location
     *
     * @param blob opaque buffer to be serialized.
     * @param size size of buffer in bytes.
     *
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool serialize(const void *blob, uint32_t size) = 0;
    /**
     * @brief Serializes a given Google Protocol Buffer to output location
     *
     * @param message Google Protocol Buffer to be serialized.
     *
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool serialize(
            const std::shared_ptr<const google::protobuf::Message>
                    &message) = 0;
    /**
     * @brief Initializes any serialization resources
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool open() = 0;
    /**
     * @brief Deinitializes any outstanding serialization resources
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual bool close() = 0;
    /**
     * @brief Returns the total number of serialized bytes
     * @retval >=0 - Number of serialized bytes
     * @retval <0 - Error code
     */
    virtual int64_t getDataSize() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACESERIALIZER_H
