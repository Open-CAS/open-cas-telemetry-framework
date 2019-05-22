/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/Log.h>

#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <typeindex>
#include <typeinfo>
#include <octf/utils/internal/Logger.h>

namespace octf {
namespace log {

OutputStream critical(std::cerr, Severity::Critical);
OutputStream cerr(std::cerr, Severity::Error);
OutputStream cout(std::cout, Severity::Information);
OutputStream verbose(std::cout, Severity::Verbose);

// [TODO](trybicki) Provide consistent implementation
// Below is a workaround for debug log
#ifdef DEBUG
OutputStream debug(std::cout, Severity::Debug);
#else
NullOutputStream debug;
#endif

OutputStream::OutputStream(std::ostream &stream, Severity severity)
        : NonCopyable()
        , m_logger(new Logger(stream, severity)) {
#ifdef DEBUG
    m_logger->enable();
#else
    if (this == &verbose) {
        m_logger->disable();
    } else {
        m_logger->enable();
    }
#endif
}

OutputStream::~OutputStream() {}

void OutputStream::setStream(std::ostream &stream) {
    m_logger->setStream(stream);
}

void setLog(Severity severity, std::ostream &stream) {
    switch (severity) {
    case Severity::Information:
        cout.setStream(stream);
        break;
    case Severity::Verbose:
        verbose.setStream(stream);
        break;
    case Severity::Critical:
        critical.setStream(stream);
        break;
#ifdef DEBUG
    case Severity::Debug:
        debug.setStream(stream);
        break;
#endif
    case Severity::Error:
        cerr.setStream(stream);
        break;
    default:
        break;
    }
}

OutputStream &OutputStream::operator<<(char value) {
    m_logger->insert(value);

    if ('\n' == value) {
        // In case when the prefix is configured, we need add it at the
        // beginning of new line. Flush will cause this.
        m_logger->flush();
    }

    return *this;
}

OutputStream &OutputStream::operator<<(unsigned char value) {
    m_logger->insert(value);

    if ('\n' == value) {
        // In case when the prefix is configured, we need add it at the
        // beginning of new line. Flush will cause this.
        m_logger->flush();
    }

    return *this;
}

OutputStream &OutputStream::operator<<(const char *value) {
    *this << std::string(value);
    return *this;
}

OutputStream &OutputStream::operator<<(const std::string &value) {
    m_logger->insert(value);

    if (!value.empty()) {
        if (value.back() == '\n') {
            m_logger->flush();
        }
    }

    return *this;
}

OutputStream &OutputStream::operator<<(std::ostream &(*fn)(std::ostream &) ) {
    m_logger->insert(fn);

    if (static_cast<std::ostream &(*) (std::ostream &)>(std::endl) == fn) {
        m_logger->flush();
    }

    return *this;
}

OutputStream &OutputStream::operator<<(OutputStreamManipulator manipulator) {
    switch (manipulator) {
    case enable:
        m_logger->enable();
        break;

    case disable:
        m_logger->disable();
        break;

    case json:
        m_logger->setJsonFormat(true);
        break;

    case text:
        m_logger->setJsonFormat(false);
        break;

    case reset:
        m_logger->setJsonFormat(false);
        m_logger->clearPrefix();
        m_logger->setPrintTimestamp(false);
        m_logger->enable();
        break;

    case timestamp:
        m_logger->setPrintTimestamp(true);
        break;

    case prefix:
        m_logger->setPrefixExpected(true);
        break;
    }

    return *this;
}

std::ostream &OutputStream::getOutputStream() {
    return m_logger->getBuffer();
}

}  // namespace log
}  // namespace octf
