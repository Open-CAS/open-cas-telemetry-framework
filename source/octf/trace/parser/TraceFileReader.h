/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEFILEREADER_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEFILEREADER_H

#include <fstream>
#include <string>
#include <octf/trace/parser/TraceFileParser.h>

namespace octf {

/**
 * @brief Helper class responsible for reading from one physical trace file
 */
class TraceFileReader {
public:
    virtual ~TraceFileReader();
    /**
     * @param filePath Path to file with traces
     */
    TraceFileReader(const std::string &filePath, uint32_t queue);

    /**
     * @brief Opens file with trace events for reading.
     *
     * @note May throw exceptions
     */
    void init();

    /**
     * @brief Closes file with trace events.
     */
    void deinit();

    /**
     * @param[out] traceEvent Event filled with trace data. Memory management
     * is fully handled by caller.
     */
    void readTraceEvent(std::shared_ptr<google::protobuf::Message> traceEvent);

    /**
     * @return Is file fully read.
     */
    bool isFinished();

    /**
     * @return Queue id associated with this reader
     */
    uint32_t getQueue() const;

private:
    /**
     * @brief Input file descriptor
     */
    int m_fd;

    /**
     * @brief Size of mapped buffer
     */
    off_t m_size;

    /**
     * @brief Input file size
     */
    off_t m_fileSize;

    /**
     * @brief Address of the mapped input file
     */
    uint8_t *m_addr;

    /**
     * @brief Path to file
     */
    std::string m_tracePath;

    /**
     * @brief Flag indicates an error occurred during parsing
     */
    bool m_error;

    /**
     * @brief Queue number
     */
    uint32_t m_queue;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEFILEREADER_H
