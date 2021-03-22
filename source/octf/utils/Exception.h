/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_EXCEPTION_H
#define SOURCE_OCTF_UTILS_EXCEPTION_H
#include <string>

namespace octf {

/**
 * @defgroup Utilities Utilities
 *
 * Generic purpose utilities
 * @{
 *
 * @defgroup Exceptions Exceptions
 * Framework Exceptions
 * @{
 */

/**q
 * @brief Exception class for iotrace framework. Base class for all other custom
 * exception types.
 *
 */
class Exception : std::exception {
public:
    Exception(const std::string &message);
    virtual ~Exception() = default;

    /**
     * @brief Exception message
     *
     * @return const char* Exception text message
     */
    virtual const char *what() const noexcept override;

    /**
     * @brief getMessage
     * @return Exception text message as a string
     */
    virtual const std::string &getMessage() const;

private:
    std::string m_message;
};

/**
 * @brief Exception for invalid parameter given ie. invalid command line input
 * or invalid function input parameter.
 *
 */
class InvalidParameterException : public Exception {
public:
    InvalidParameterException(const std::string &message)
            : Exception(message) {}
    virtual ~InvalidParameterException() = default;
};

/**
 * @brief Protobuffer specific exception.
 * Contains status codes defined in google/rpc/code.proto.
 *
 */
class ProtoBufferException : public Exception {
public:
    ProtoBufferException(const std::string &message)
            : Exception(message) {}
    virtual ~ProtoBufferException() = default;
};

/**
 * @brief Exception for invalid node id
 *
 */
class InvalidNodeIdException : public InvalidParameterException {
public:
    InvalidNodeIdException(const std::string &message)
            : InvalidParameterException(message) {}
    virtual ~InvalidNodeIdException() = default;
};

/**
 * @brief Exception which occurred during shell execution
 */
class ShellExecutorException : public octf::Exception {
public:
    ShellExecutorException(const std::string &message, const std::string &cmd)
            : Exception(std::string("Exception occurred during '") + cmd +
                        "' execution: " + message) {}
    virtual ~ShellExecutorException() = default;
};

/**
 * @brief Exception which occurred during file system path traversing
 */
class MaxPathExceededException : public octf::Exception {
public:
    MaxPathExceededException(const uint64_t inode)
            : Exception(
                      std::string(
                              "Exceeded maximum path length limit for inode ") +
                      std::to_string(inode)) {}
    virtual ~MaxPathExceededException() = default;
};

/**
 * @}
 * @}
 */

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_EXCEPTION_H
