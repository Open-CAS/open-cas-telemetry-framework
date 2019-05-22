/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H
#define SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H

#include <google/protobuf/message.h>
#include <fstream>
#include <octf/node/NodeId.h>

namespace octf {

/**
 * @brief Protocol buffer message reader/writer from/to file
 */
class ProtobufReaderWriter {
public:
    /**
     * @param filePath File location where protocol buffer message will be
     * written or read
     */
    ProtobufReaderWriter(const std::string &filePath);
    virtual ~ProtobufReaderWriter() = default;

    /**
     * @brief Checks if file exists and checks for access rights
     *
     * @param access Access rights to check as unistd.h defines
     * @return Availability of file
     */
    bool isFileAvailable(std::fstream::openmode access = std::fstream::in |
                                                         std::fstream::out);

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

private:
    /**
     * File path
     */
    std::string m_filePath;

    /**
     * Directory location with specified file
     */
    std::string m_directoryPath;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_PROTOBUFREADERWRITER_H
