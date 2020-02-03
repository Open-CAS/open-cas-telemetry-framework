/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_SHELLEXECUTOR_H
#define SOURCE_OCTF_UTILS_SHELLEXECUTOR_H

#include <string>

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
	std::string executeCommand();

	const std::string& getArgs() const;

	const std::string& getCommand() const;

	void setCommand(const std::string &command);

	void setArgs(const std::string &args);

private:
	std::string m_command;
	std::string m_args;

};
} // namespace octf

#endif // SOURCE_OCTF_UTILS_SHELLEXECUTOR_H
