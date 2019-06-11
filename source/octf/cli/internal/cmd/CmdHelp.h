/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CMD_CMDHELP_H
#define SOURCE_OCTF_CLI_CMD_CMDHELP_H

#include <string>
#include <octf/cli/internal/cmd/Command.h>
namespace octf {

/**
 * @brief Help command
 *
 * This command shows general help for the application
 */
class CmdHelp : public Command {
public:
    /**
     * @brief Command constructor
     */
    CmdHelp();

    virtual ~CmdHelp() = default;

    void execute() override;

    void setHelp(const std::string &help);

private:
    std::string m_help;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_CMD_CMDHELP_H
