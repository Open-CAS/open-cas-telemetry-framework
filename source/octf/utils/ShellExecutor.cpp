/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/ShellExecutor.h>
#include <octf/utils/Exception.h>
#include <cstdio>
#include <memory>
#include <array>

namespace octf {


std::string ShellExecutor::execute() {
	std::string command = m_command;
	command += " ";
	command += m_args;

	const char* cmd = command.c_str();

	std::array<char, 128> buffer;
	std::string result;

	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw ShellExecutorException("call to popen() failed", m_command);
	}

	int read;
	while ((read = fread(buffer.data(), 1, buffer.size(), pipe.get()))
			== static_cast<int>(buffer.size())) {
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

	return result;
}

} // namespace octf
