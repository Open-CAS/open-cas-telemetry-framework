/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_SHELLEXECUTOR_H
#define SOURCE_OCTF_UTILS_SHELLEXECUTOR_H

#include <string>
#include <octf/utils/Exception.h>

namespace octf {

/**
 * @brief Class for executing shell commands
 */
class ShellExecutor {
public:
	ShellExecutor() = default;
	virtual ~ShellExecutor() = default;

	/**
	 * @brief Executes a command in shell by using popen()
	 *
	 * This effectively creates a pipe, forks and invokes the default shell
	 *
	 * @note Command and arguments are simply concatenated with spaces
	 */
	std::string execute();

	const std::string& getArgs() const {
		return m_args;
	}

	const std::string& getCommand() const {
		return m_command;
	}

	void setCommand(const std::string &command) {
		m_command = command;
	}

	void setArgs(const std::string &args) {
		m_args = args;
	}

private:
	std::string m_command;
	std::string m_args;

};

} // namespace octf

#endif // SOURCE_OCTF_UTILS_SHELLEXECUTOR_H
