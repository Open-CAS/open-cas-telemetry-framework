/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/trace/parser/TraceFileReader.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <octf/interface/TraceManager.h>
#include <octf/trace/parser/TraceFileParser.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/ProtoConverter.h>

namespace octf {

TraceFileReader::~TraceFileReader() {
    deinit();
}

TraceFileReader::TraceFileReader(const std::string &filePath, uint32_t queue)
        : m_fd(-1)
        , m_size()
        , m_fileSize()
        , m_addr(nullptr)
        , m_tracePath(filePath)
        , m_error(false)
        , m_queue(queue) {}

void TraceFileReader::init() {
    if (m_fd < 0) {
        struct stat fileStats;
        int flags = O_RDONLY;
        int proto = PROT_READ;

        m_fd = open(m_tracePath.c_str(), flags,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (m_fd < 0) {
            throw Exception("Could not open trace file." + m_tracePath);
        }

        if (fstat(m_fd, &fileStats)) {
            close(m_fd);
            throw Exception("Could not get trace file status." + m_tracePath);
        }

        m_fileSize = m_size = fileStats.st_size;
        if (!m_size) {
            // Empty trace
            return;
        }

        m_addr = (uint8_t *) mmap(NULL, m_size, proto, MAP_SHARED, m_fd, 0);
        if (MAP_FAILED == m_addr) {
            m_addr = NULL;
            close(m_fd);
            throw Exception("Could not map trace file.");
        }
    }
}

void TraceFileReader::deinit() {
    if (m_fd >= 0) {
        if (m_addr && MAP_FAILED != m_addr) {
            munmap(m_addr, m_fileSize);
            m_addr = nullptr;
        }
        close(m_fd);
        m_fd = -1;
    }
}

void TraceFileReader::readTraceEvent(
        std::shared_ptr<google::protobuf::Message> traceEvent) {
    if (m_error) {
        throw Exception("Attempted to read from parser which had failed");
    }

    if (isFinished()) {
        throw Exception("Attempted to read from fully parsed file");
    }

    // Decode length of trace event
    int messageLength = 0;
    int bytesRead =
            protoconverter::decodeVarint32(m_addr, m_size, messageLength);
    if (bytesRead <= 0 || bytesRead > m_size) {
        m_error = true;
        throw Exception("Couldn't parse size of trace event size");
    }
    m_addr += bytesRead;
    m_size -= bytesRead;

    // Parse trace event
    if (false == traceEvent->ParseFromArray(m_addr, messageLength) ||
        messageLength > m_size) {
        m_error = true;
        throw Exception("Couldn't parse valid trace event");
    }
    m_addr += messageLength;
    m_size -= messageLength;
}

bool TraceFileReader::isFinished() {
    if (m_size) {
        return false;
    }
    return true;
}

}  // namespace octf
