/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CMD_CMDVERSION_H
#define SOURCE_OCTF_CLI_CMD_CMDVERSION_H

#include <octf/cli/CLIProperties.h>
#include <octf/cli/cmd/Command.h>

namespace octf {

/**
 * @brief Command for displaying application version
 */
class CmdVersion : public Command {
public:
    /**
     * @brief Command constructor
     */
    CmdVersion(const CLIProperties &properties);

    virtual ~CmdVersion() = default;

    /**
     * @brief Executes command
     */
    void execute() override;

private:
    const CLIProperties m_cliProperties;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_CMD_CMDVERSION_H
