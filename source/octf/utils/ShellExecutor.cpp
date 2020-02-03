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


std::string ShellExecutor::executeCommand() {
	std::string command = m_command;
	command += " ";
	command += m_args;

	const char* cmd = command.c_str();

	std::array<char, 128> buffer;
	std::string result;

	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw Exception("Call to popen() failed!");
	}

	int read;
	while ((read = fread(buffer.data(), 1, buffer.size(), pipe.get()))
			== static_cast<int>(buffer.size())) {
		result += buffer.data();
	}

	// Error occurred
	if (ferror(pipe.get())) {
		throw Exception("Could not read from started casadm process");
	}

	// Not entire buffer was read at the end
	if (feof(pipe.get()) && read > 0) {
		buffer[read] = '\0';
		result += buffer.data();
	}

	// This should never occur
	if (!feof(pipe.get())) {
		throw Exception("Invalid result of reading from process");
	}

	return result;
}

const std::string& ShellExecutor::getArgs() const {
	return m_args;
}

const std::string& ShellExecutor::getCommand() const {
	return m_command;
}

void ShellExecutor::setCommand(const std::string &command) {
	m_command = command;
}

void ShellExecutor::setArgs(const std::string &args) {
	m_args = args;
}

} // namespace octf
