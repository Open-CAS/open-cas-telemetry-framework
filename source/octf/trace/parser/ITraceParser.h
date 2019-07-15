/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_ITRACEPARSER_H
#define SOURCE_OCTF_TRACE_PARSER_ITRACEPARSER_H

#include <google/protobuf/message.h>
#include <string>

namespace octf {

/**
 * @brief General trace parsing operations.
 *
 * Specific implementations (derived classes) can provide parsing for
 * for different trace sources (filesystem, database)
 */
class ITraceParser {
public:
    /**
     * @brief Constructs parser
     */
    ITraceParser() = default;
    virtual ~ITraceParser() = default;
    /**
     * @brief Initializes parser and all its necessary resources
     *
     * @note May throw exceptions
     * @note Depending of specific types, initialization can be e.g. opening
     * file, establishing connection with database
     */
    virtual void init() = 0;
    /**
     * @brief Deinitializes parser and its resources (e.g. closes files).
     */
    virtual void deinit() = 0;
    /**
     * @brief Parses (next) single trace event into provided prototype.
     *
     * @param[out] traceEvent Prototype of Message to be filled with parsed data
     * of trace event
     *
     * @throws In case an error Exception shall be raised
     */
    virtual void parseTraceEvent(google::protobuf::Message *traceEvent) = 0;
    /**
     * @brief Checks if whole trace has been parsed.
     */
    virtual bool isFinished() = 0;
};
}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_ITRACEPARSER_H
