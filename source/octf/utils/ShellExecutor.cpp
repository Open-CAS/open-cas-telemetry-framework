/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <errno.h>
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>
#include <octf/utils/Exception.h>
#include <octf/utils/ShellExecutor.h>

namespace octf {

std::string ShellExecutor::execute() {
    std::string result;

    std::string command = m_command;
    command += " ";
    command += m_args;

    const char *cmd = command.c_str();

    std::array<char, 128> buffer;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw ShellExecutorException("call to popen() failed", m_command);
    }

    int read;
    while ((read = fread(buffer.data(), 1, buffer.size(), pipe.get())) ==
           static_cast<int>(buffer.size())) {
        result.append(buffer.data(), read);
    }

    if (ferror(pipe.get())) {
        throw ShellExecutorException("could not read from started process",
                                     m_command);
    }

    // Not entire buffer was read at the end
    if (feof(pipe.get()) && read > 0) {
        result.append(buffer.data(), read);
    }

    // This should never occur
    if (!feof(pipe.get())) {
        throw ShellExecutorException("invalid result of reading from process",
                                     m_command);
    }

    // Now check program return code
    auto pipePtr = pipe.release();
    int retCode = pclose(pipePtr);
    if (retCode) {
        throw ShellExecutorException(std::strerror(errno), m_command);
    }

    return result;
}

}  // namespace octf
