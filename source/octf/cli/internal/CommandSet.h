/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_COMMANDSET_H
#define SOURCE_OCTF_CLI_INTERNAL_COMMANDSET_H

#include <map>
#include <memory>
#include <string>
#include <octf/cli/internal/ICommandSet.h>
#include <octf/cli/internal/cmd/ICommand.h>

namespace octf {
namespace cli {

class CmdHelp;

/**
 * @brief Set of available commands
 */
class CommandSet : ICommandSet {
public:
    /**
     * @brief Default constructor
     */
    CommandSet();

    virtual ~CommandSet() = default;

    /**
     * @brief Gets command from existing command set
     *
     * @param cmd Command name to be found in command set
     * @return Command
     */
    virtual std::shared_ptr<ICommand> getCmd(std::string cmd) override;

    /**
     * @brief Returns generated help for all available commands
     */
    virtual void getHelp(std::stringstream &ss) const override;

    /**
     * @brief Adds command to command set
     *
     * @param cmd Command to be added
     */
    virtual void addCmd(std::shared_ptr<ICommand> cmd) override;

    /**
     * @brief Removes all commands from command set
     */
    virtual void clear();

    /**
     * @brief Checks if there exists a command with given command name
     * @param cmdName Name of command (short o long key)
     * @return Is corresponding command present in command set
     */
    virtual bool hasCmd(const std::string &cmdName) const;

    /**
     * @brief Sets content of command set's help command output
     * @param help Help output to be set
     */
    virtual void setHelpCmdContent(const std::string &help);

    /**
     * @return Pointer to help command
     */
    std::shared_ptr<CmdHelp> getHelpCmd() const;

    virtual void updateHelp();

private:
    std::shared_ptr<CmdHelp> m_helpCmd;
    std::map<std::string, std::shared_ptr<ICommand>> m_cmds;
    std::map<std::string, std::shared_ptr<ICommand>>::iterator m_iter;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_COMMANDSET_H
