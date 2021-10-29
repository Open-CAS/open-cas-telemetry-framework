/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CMD_CMDVERSION_H
#define SOURCE_OCTF_CLI_INTERNAL_CMD_CMDVERSION_H

#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/cmd/Command.h>

namespace octf {
namespace cli {

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
    const CLIProperties &m_cliProperties;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CMD_CMDVERSION_H
