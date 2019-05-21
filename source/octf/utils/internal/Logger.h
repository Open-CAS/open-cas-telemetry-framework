/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_INTERNAL_LOGGER_H
#define SOURCE_OCTF_UTILS_INTERNAL_LOGGER_H

#include <google/protobuf/util/json_util.h>
#include <memory>
#include <ostream>
#include <sstream>
#include <octf/proto/defs.pb.h>
#include <octf/utils/DateTime.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace log {

/**
 * This class executes logging logic.
 *
 * It assures thread-safety, prefix insertion, etc...
 */
class Logger : public NonCopyable {
public:
    Logger(std::ostream &stream, Severity severity)
            : NonCopyable()
            , m_severity(severity)
            , m_stream(stream)
            , m_enabled(false)
            , m_prefix("")
            , m_prefixExpected(false)
            , m_printTimestamp(false)
            , m_jsonFormat(false){};

    virtual ~Logger() = default;

    template <typename Type>
    inline void insert(Type value) {
        if (m_enabled) {
            getBuffer() << value;
        }
        m_prefixExpected = false;
    }

    inline void insert(const std::string &value) {
        if (m_prefixExpected) {
            m_prefix = value;
        } else {
            if (m_enabled) {
                getBuffer() << value;
            }
        }
        m_prefixExpected = false;
    }

    void flush() {
        if (m_enabled) {
            m_stream.get() << getLog();
        }
    }

    void setStream(std::ostream &stream) {
        m_stream = stream;
    }

    void disable() {
        m_enabled = false;
    }

    void enable() {
        m_enabled = true;
    }

    void clearPrefix() {
        m_prefix = "";
        m_prefixExpected = false;
    }

    void setPrintTimestamp(bool printTimestamp) {
        m_printTimestamp = printTimestamp;
    }

    void setJsonFormat(bool jsonFormat) {
        m_jsonFormat = jsonFormat;
    }

    void setPrefixExpected(bool prefixWaitFor) {
        m_prefixExpected = prefixWaitFor;
    }

    /**
     * Get buffer for current thread and severity
     *
     * @return buffer which is string stream
     */
    std::stringstream &getBuffer() {
        switch (m_severity) {
        case Severity::Information: {
            static thread_local std::stringstream buffer;
            return buffer;
        }

        case Severity::Verbose: {
            static thread_local std::stringstream buffer;
            return buffer;
        }

        case Severity::Critical: {
            static thread_local std::stringstream buffer;
            return buffer;
        }

#ifdef DEBUG
        case Severity::Debug: {
            static thread_local std::stringstream buffer;
            return buffer;
        }
#endif

        case Severity::Error: {
            static thread_local std::stringstream buffer;
            return buffer;
        }

        default:
            throw Exception("Unknown log severity type");
        }
    }

private:
    inline std::string getLog() {
        if (m_jsonFormat) {
            return getJsonLog();
        } else {
            return getTxtLog();
        }
    }

    inline std::string getTxtLog() {
        bool space = false;
        std::stringstream trace;

        if (m_prefix.length()) {
            trace << m_prefix;
            space = true;
        }

        if (m_printTimestamp) {
            if (space) {
                trace << " ";
            }
            trace << "[" << datetime::getTimestamp() << "]";
            space = true;
        }

        if (space) {
            trace << " ";
        }

        auto &buffer = getBuffer();
        trace << buffer.str();
        buffer.str("");

        return trace.str();
    }

    inline std::string getJsonLog() {
        proto::LogTrace trace;

        trace.set_timestamp(datetime::getTimestamp());

        auto &buffer = getBuffer();
        std::string str = buffer.str();
        buffer.str("");

        // erase new line char
        auto length = str.length();
        if (length) {
            str = str.substr(0, length - 1);
        }

        trace.set_trace(str);
        trace.set_system(m_prefix);

        switch (m_severity) {
        case Severity::Information:
            trace.set_severity(proto::LogTrace_Severity_Information);
            break;
        case Severity::Verbose:
            trace.set_severity(proto::LogTrace_Severity_Verbose);
            break;
        case Severity::Critical:
            trace.set_severity(proto::LogTrace_Severity_Critical);
            break;
        case Severity::Error:
            trace.set_severity(proto::LogTrace_Severity_Error);
            break;
        case Severity::Debug:
            trace.set_severity(proto::LogTrace_Severity_Debug);
            break;
        default:
            trace.set_severity(proto::LogTrace_Severity_Undefined);
            break;
        }

        google::protobuf::util::Status status;
        google::protobuf::util::JsonPrintOptions opts;
        std::string result;

        opts.add_whitespace = true;
        opts.always_print_primitive_fields = false;

        status = google::protobuf::util::MessageToJsonString(trace, &result,
                                                             opts);
        if (!status.ok()) {
            return "Cannot generate trace in JSON format";
        };

        return result;
    }

private:
    /**
     * Severity of this logger
     */
    Severity m_severity;

    /**
     * Final output stream
     */
    std::reference_wrapper<std::ostream> m_stream;

    /**
     * Flag indicates logger activity.
     */
    bool m_enabled;

    /**
     * Prefix which will be added to each log trace (line)
     */
    std::string m_prefix;

    /**
     * Flag indicates that prefix manipulator has been called and now logger
     * is waiting for prefix string. After setting prefix, for each line it will
     * be added.
     */
    bool m_prefixExpected;

    /**
     * Flag indicates that for each trace (output line) date time stamp will be
     * added. This flag is valid only for text mode
     */
    bool m_printTimestamp;

    /**
     * Flag indicates that trace format shall be JSON
     */
    bool m_jsonFormat;
};

}  // namespace log
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_INTERNAL_LOGGER_H
