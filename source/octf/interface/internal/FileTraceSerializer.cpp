/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <third_party/safestringlib.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <octf/interface/internal/FileTraceSerializer.h>
#include <octf/utils/Exception.h>
#include <octf/utils/ProtoConverter.h>
#include <octf/utils/SizeConversion.h>

namespace octf {

FileTraceSerializer::FileTraceSerializer(const std::string &outputFileName)
        : m_fd(-1)
        , m_addr(NULL)
        , m_dataOffset(0)
        , m_fileOffset(0)
        , m_size(0)
        , m_outputFileName(outputFileName)
        , m_pageSize(sysconf(_SC_PAGE_SIZE)) {}

FileTraceSerializer::~FileTraceSerializer() {
    close();
}

bool FileTraceSerializer::open() {
    if (m_fd < 0) {
        int flags = O_CREAT | O_RDWR;

        m_fd = ::open(m_outputFileName.c_str(), flags,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (m_fd < 0) {
            return false;
        }
    }
    return true;
}

void FileTraceSerializer::moveDataPointer(uint64_t size) {
    m_dataOffset += size;
}

bool FileTraceSerializer::close() {
    bool success = false;
    if (m_fd >= 0) {
        int errorCode = ftruncate(m_fd, m_dataOffset);
        success = !errorCode;
        if (m_addr) {
            munmap(m_addr, m_size);
            m_addr = nullptr;
        }
        errorCode = ::close(m_fd);
        m_fd = -1;
        success &= !errorCode;
    }
    return success;
}

bool FileTraceSerializer::serialize(const void *blob, uint32_t size) {
    auto buffer = getBuffer(size);

    int64_t destSize = m_size - (m_dataOffset - m_fileOffset);
    if (destSize < 0) {
        Exception("File trace serializer buffer error");
    }

    memcpy_s(buffer, destSize, blob, size);

    moveDataPointer(size);

    return true;
}

bool FileTraceSerializer::serialize(
        const std::shared_ptr<const google::protobuf::Message> &message) {
    using namespace google::protobuf::io;

    if (!message) {
        return false;
    }

    int messageLength = message->ByteSize();

    uint8_t *buffer = getBuffer(protoconverter::MAX_VARINT32_BYTES);
    if (!buffer) {
        return false;
    }

    int bytesWritten = protoconverter::encodeVarint32(
            buffer, protoconverter::MAX_VARINT32_BYTES, messageLength);
    if (bytesWritten > 0) {
        moveDataPointer(bytesWritten);
    } else {
        return false;
    }

    buffer = getBuffer(messageLength);
    if (!buffer) {
        return false;
    }

    if (message->SerializeToArray(buffer, messageLength)) {
        moveDataPointer(messageLength);
        return true;
    }

    return false;
}

uint8_t *FileTraceSerializer::getBuffer(size_t size) {
    uint64_t offset = m_dataOffset;
    if (isRemapNeeded(offset, size)) {
        // Remap at least 1 MiB, to minimize kernel calls
        size = std::max(MiBToBytes(1), size);
        remap(offset, size);
    }

    offset -= m_fileOffset;

    return static_cast<uint8_t *>(m_addr) + offset;
}

bool FileTraceSerializer::isRemapNeeded(uint64_t offset, uint32_t size) {
    uint64_t mappedBegin = m_fileOffset;
    uint64_t mappedEnd = m_fileOffset + m_size - 1;

    uint64_t begin = offset;
    uint64_t end = offset + size - 1;

    if (!m_addr) {
        return true;
    }

    if (begin < mappedBegin || begin > mappedEnd) {
        return true;
    }

    if (end < mappedBegin || end > mappedEnd) {
        return true;
    }

    return false;
}

void FileTraceSerializer::remap(uint64_t offset, uint32_t size) {
    int proto = 0;
    uint64_t pageOffset = offset / m_pageSize;
    uint64_t pages =
            (size + m_pageSize - 1 + (offset % m_pageSize)) / m_pageSize;

    if (m_addr) {
        munmap(m_addr, m_size);
        m_addr = NULL;
    }

    m_fileOffset = pageOffset * m_pageSize;
    m_size = pages * m_pageSize;
    if (posix_fallocate(m_fd, m_fileOffset, m_size)) {
        throw Exception("Cannot allocate space, file " + m_outputFileName);
    }
    proto = PROT_READ | PROT_WRITE;

    m_addr = mmap(NULL, m_size, proto, MAP_SHARED, m_fd, m_fileOffset);
    if (MAP_FAILED == m_addr) {
        m_addr = NULL;
        throw Exception("Cannot map file " + m_outputFileName);
    }

    if (isRemapNeeded(offset, size)) {
        throw Exception("Failed to properly map file " + m_outputFileName);
    }
}

}  // namespace octf
