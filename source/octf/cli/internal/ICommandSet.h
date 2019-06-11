/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_ICOMMANDSET_H
#define SOURCE_OCTF_CLI_ICOMMANDSET_H

#include <map>
#include <memory>
#include <string>
#include <octf/cli/internal/cmd/ICommand.h>

namespace octf {

/**
 * @brief Set of available commands
 *
 * This class is used for storing and exposing commands supported
 * by module (e.g. plugin)
 */
class ICommandSet {
public:
    /**
     * @brief Default constructor
     */
    ICommandSet() = default;

    /**
     * @brief Destructor
     */
    virtual ~ICommandSet() = default;

    /**
     * @brief Gets command from existing command set
     *
     * @param cmd Command name or key to be found in command set
     * @return Command
     *
     * Function can return reference to internal object.
     * Thus, it mustn't be used after CommandSet's destruction.
     */
    virtual std::shared_ptr<ICommand> getCmd(std::string cmd) = 0;

    /**
     * @brief Gets help for all available commands
     */
    virtual void getHelp(std::stringstream &ss) const = 0;

    /**
     * @brief Adds command to command set
     *
     * @param cmd Command to be added
     */
    virtual void addCmd(std::shared_ptr<ICommand> cmd) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_ICOMMANDSET_H
