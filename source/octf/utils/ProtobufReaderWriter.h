/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H
#define SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H

#include <errno.h>
#include <google/protobuf/message.h>
#include <octf/node/NodeId.h>

namespace octf {

/**
 * @brief Protocol buffer message reader/writer from/to file
 *
 * @note This class is designed to avoid TOCTOU (Time Of Check, Time Of
 *  Use) vulnerability.
 */
class ProtobufReaderWriter {
public:
    /**
     * @param filePath File location where protocol buffer message will be
     * written or read
     *
     * @note This constructor may throw an Exception
     */
    ProtobufReaderWriter(const std::string &filePath);

    virtual ~ProtobufReaderWriter();

    /**
     * @brief Checks if file exists and checks if is a regular file
     *
     * @return Availability of file
     */
    bool isFileAvailable();

    /**
     * @brief Deserialize a message from file
     *
     * @param[out] message Message to be read
     * @return Result of operation
     */
    bool read(google::protobuf::Message &message);

    /**
     * @brief Serialize a message to file
     *
     * @param message Message to be written
     * @return Result of operation
     */
    bool write(const google::protobuf::Message &message);

    /**
     * @brief Remove file
     *
     * @return Result of operation
     */
    bool remove();

    /**
     * @brief Get file path corresponding to this ReaderWriter
     *
     * @return File path
     */
    const std::string &getFilePath() const {
        return m_filePath;
    }

    /**
     * @brief Sets permission flags on opened file to make it read only for
     * user and group.
     *
     * @return Result of operation
     */
    bool makeReadOnly();

    /**
     * @brief Sets permission flags on opened file to make it writable for
     * user and group.
     *
     * @return Result of operation
     */
    bool makeWritable();

    /**
     * @brief Checks if printing errors is enabled
     * @retval true Errors messages will be printed
     * @retval false Errors message will be suppressed
     */
    bool isVerbose() const {
        return m_verbose;
    }

    /**
     * @brief Enables printing error message
     *
     * @param verbose Flags controlling printing error messages. If verbose true
     * then error messages will be printed. Otherwise not.
     */
    void setVerbose(bool verbose) {
        m_verbose = verbose;
    }

    /**
     * @brief Locks the file associated with this ReaderWriter
     *
     * @note The lock mechanism is based on advisory kind of file lock. If a
     * caller calls read/write without locking, it will be allowed. To make
     * sure access synchronization works, each invocation of read and write
     * needs to be preceded by calling lock.
     */
    void lock();

    /**
     * @brief Unlocks the file associated with this ReaderWriter
     */
    void unlock();

    /**
     * @brief Checks if the file associated is empty
     *
     * @retval true File is empty and no writes in the past
     * @retval false File is empty and there had beed writes in the past
     */
    bool isEmpty();

    /**
     * @brief Clears the content of the file
     *
     * @return Operation result
     */
    bool clear();

private:
    /**
     * @brief This opens the file and gets a file descriptor for reading.
     *
     * @note This function does not open symlinks, and throws an exception
     * upon doing so.
     */
    void openFileToRead();

    /**
     * @brief This opens the file and gets a file descriptor for writing.
     *
     * @note This function does not open symlinks, and throws an exception
     * upon doing so.
     */
    void openFileToWrite();

    /**
     * @brief This closes the opened file. It is called upon destruction.
     */
    void closeFile();

private:
    /**
     * File path
     */
    std::string m_filePath;

    /**
     * Directory location with specified file
     */
    std::string m_directoryPath;

    /**
     * File descriptor for reading
     */
    int m_readFd;

    /**
     * File descriptor for writing
     */
    int m_writeFd;

    /**
     * @brief File descriptor for locking between processes
     */
    int m_lockFd;

    /**
     * @brief Lock for locking between threads
     */
    std::unique_ptr<std::lock_guard<std::mutex>> m_lock;

    /**
     * Flags to control if print error
     */
    bool m_verbose;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H
