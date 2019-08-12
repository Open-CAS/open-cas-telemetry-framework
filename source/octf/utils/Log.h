/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_LOG_H
#define SOURCE_OCTF_UTILS_LOG_H

#include <functional>
#include <iostream>
#include <memory>
#include <streambuf>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace log {

/**
 * @brief Severity of output stream
 *
 */
enum class Severity {
    /**
     * Critical errors, resulting in program's aborting or restarting
     */
    Critical = 0,

    /**
     * Non-critical errors; no need for stopping or restarting program
     */
    Error,

    /**
     * Important information about normal program's execution
     */
    Information,

    /**
     * Extended information available when verbose logging is enabled
     *
     * @note in contrast to Debug, Verbose is available in release build and
     * inactive
     */
    Verbose,

    /**
     * All debug information
     *
     * @note In release build, debug logs are stripped
     */
    Debug,
};

class Logger;

/**
 * Manipulator for OutputStream
 */
enum OutputStreamManipulator {
    /**
     * Enables stream
     */
    enable,

    /**
     * Disables stream
     */
    disable,

    /**
     * Configures stream to print in JSON format
     */
    json,

    /**
     * Configures stream to print in regular text format
     */
    text,

    /**
     * Resets stream to default configuration
     */
    reset,

    /**
     * Configures stream to print additional time stamp for each line
     */
    timestamp,

    /**
     * Configures stream to print specified prefix for each line
     *
     * @code
     * log::cout << log::prefix << "YOUR PREFIX";
     * @endcode
     */
    prefix
};

/**
 * @brief Custom output stream class basing on standard stream implementation
 *
 */
class OutputStream : public NonCopyable {
public:
    /**
     * @brief Constructs a new Output Stream object
     *
     * @param stream Default stream implementation
     */
    OutputStream(std::ostream &stream, Severity severity);

    virtual ~OutputStream();

    /**
     * @brief Sets selected stream object for given stream
     *
     * @param stream Stream to set for
     */
    void setStream(std::ostream &stream);

    OutputStream &operator<<(char value);
    OutputStream &operator<<(unsigned char value);
    OutputStream &operator<<(const char *value);
    OutputStream &operator<<(const std::string &value);
    OutputStream &operator<<(std::ostream &(*fn)(std::ostream &) );
    OutputStream &operator<<(OutputStreamManipulator manipulator);

    template <typename Type>
    OutputStream &operator<<(const Type &value) {
        getOutputStream() << value;
        return *this;
    }

    /**
     * @Gets stream prefix
     *
     * @return Stream prefix
     */
    const std::string &getPrefix() const;

    /**
     * @brief Gets log severity
     *
     * @return Log severity
     */
    Severity getSeverity() const;

private:
    std::ostream &getOutputStream();

private:
    std::unique_ptr<Logger> m_logger;
};

class NullOutputStream {
public:
    NullOutputStream() = default;
    virtual ~NullOutputStream() = default;

    template <typename Type>
    inline NullOutputStream &operator<<(Type __attribute__((__unused__))) {
        return *this;
    }
    NullOutputStream &operator<<(std::ostream &(*fn)(std::ostream &)
                                         __attribute__((__unused__))) {
        return *this;
    }
};

/**
 * @brief Standard output stream
 *
 */
extern OutputStream cout;

/**
 * @brief Verbose output stream
 *
 */
extern OutputStream verbose;

/**
 * @brief Error output stream
 *
 */
extern OutputStream cerr;

/**
 * @brief Debug stream for development purposes
 *
 */
#ifdef DEBUG
extern OutputStream debug;
#else
extern NullOutputStream debug;
#endif
/**
 * @brief Critical information stream
 *
 */
extern OutputStream critical;

/**
 * @brief Sets the Stream object to given stream depending on severity
 *
 * @param severity Severity of log
 * @param stream Stream to set
 */
void setLog(Severity severity, std::ostream &stream);

}  // namespace log
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_LOG_H
