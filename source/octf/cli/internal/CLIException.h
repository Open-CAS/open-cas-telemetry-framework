/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CLIEXCEPTION_H
#define SOURCE_OCTF_CLI_INTERNAL_CLIEXCEPTION_H

#include <string>
#include <octf/utils/Exception.h>

namespace octf {
namespace cli {

/**
 * @brief CLI execution failure
 *
 * Handling exception is supposed to:
 * - print message on standard error
 * - print help message on standard output if not empty
 * - exit program with result defined by failure parameter
 */
class CLIException : public Exception {
public:
    /**
     * @param message Error message to be printed on error output
     * @param help Help to be printed on standard output
     * @param failure Result of CLI program
     */
    CLIException(const std::string &message, bool help, bool failure)
            : Exception(message)
            , m_printHelp(help)
            , m_failure(failure) {}

    virtual ~CLIException() = default;

    bool isFailure() const {
        return m_failure;
    }

    void setFailure(bool failure) {
        m_failure = failure;
    }

    bool printHelp() const {
        return m_printHelp;
    }

    void setPrintHelp(bool help) {
        m_printHelp = help;
    }

private:
    bool m_printHelp;
    bool m_failure;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CLIEXCEPTION_H
