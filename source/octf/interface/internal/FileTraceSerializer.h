/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERNAL_FILETRACESERIALIZER_H
#define SOURCE_OCTF_INTERFACE_INTERNAL_FILETRACESERIALIZER_H

#include <string>
#include <octf/interface/ITraceSerializer.h>

namespace octf {

/**
 * @brief Class for management of a trace file, including mapping parts of it
 */
class FileTraceSerializer : public ITraceSerializer {
public:
    FileTraceSerializer(const std::string &outputFileName);
    ~FileTraceSerializer();

    bool open() override;

    bool close() override;

    int64_t getDataSize() override {
        return m_dataOffset;
    }

    bool serialize(const void *blob, uint32_t size) override;

    bool serialize(const std::shared_ptr<const google::protobuf::Message>
                           &message) override;

    bool serialize(const google::protobuf::Message &message) override;

private:
    bool isRemapNeeded(uint64_t offset, uint32_t size);
    void remap(uint64_t offset, uint32_t size);
    /**
     * @brief Returns a buffer of given size from the mapped
     * file
     */
    uint8_t *getBuffer(size_t size);
    /**
     * @brief Advances the internal data write pointer by
     * given amount
     */
    void moveDataPointer(uint64_t size);

    /**
     * @brief Output file descriptor
     */
    int m_fd;
    /**
     * @brief Address of the currently mapped part of the output file
     */
    void *m_addr;
    /**
     * @brief Current offset (within the file) of the data write pointer
     */
    int64_t m_dataOffset;
    /**
     * @brief Current offset (within the file) of the mapped address
     */
    int64_t m_fileOffset;
    /**
     * @brief Total file size
     */
    int64_t m_size;
    const std::string m_outputFileName;
    /**
     * Page size as reported by the system (for mmap purposes)
     */
    const uint64_t m_pageSize;
};
}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERNAL_FILETRACESERIALIZER_H
