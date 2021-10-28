/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CMD_ICOMMAND_H
#define SOURCE_OCTF_CLI_INTERNAL_CMD_ICOMMAND_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <octf/cli/internal/param/IParameter.h>
#include <octf/interface/InterfaceId.h>
#include <octf/utils/Log.h>

namespace octf {
namespace cli {

class CLIList;

/**
 * @brief Interface of any CLI command
 *
 * This interface specifies any CLI command that is to be created.
 * A command can have any number of parameters describing it
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @return Short key of command
     */
    virtual const std::string &getShortKey() const = 0;

    /**
     * @return Long key of command
     */
    virtual const std::string &getLongKey() const = 0;

    /**
     * @return Description of command
     */
    virtual const std::string &getDesc() const = 0;

    /**
     * @return Number of parameters this command takes
     */
    virtual uint32_t getParamsCount() const = 0;

    /**
     * @brief Gets parameter for this command using name
     *
     * @param name Name of the parameter
     */
    virtual std::shared_ptr<IParameter> getParam(const std::string &name) = 0;

    /**
     * @brief Gets parameter for this command using index
     *
     * @param index Index of the parameter
     *
     * @retval Parameter with given index
     */
    virtual std::shared_ptr<IParameter> getParamByIndex(
            const int32_t index) = 0;

    /**
     * @brief Parse parameters' values from CLIList
     *
     * @throw Exception
     * If parsing failed, e.g. required parameter is missing.
     */
    virtual void parseParamValues(CLIList &cliList) = 0;

    /**
     *  @brief Gets help for this command
     *
     *  Help shows application usage if this command is not
     *  a subcommand, then all parameters for this command are listed
     */
    virtual void getHelp(std::stringstream &ss) const = 0;

    /**
     * @brief Return a short message defining usage of command
     *
     * @note Usage is different from help in that it
     * shows only correct usage pattern of the command e.g.
     * "Usage: ./app_name --requiredParameter <PARAMETER_CONTENT> [option...]"
     */
    virtual void getCommandUsage(std::stringstream &ss) const = 0;

    /**
     * @brief Check if any of the required parameters are missing
     *
     * @throw CliException if a parameter is missing
     */
    virtual void checkParamMissing() const = 0;

    /**
     * @param key Short key of the command
     */
    virtual void setShortKey(const std::string &key) = 0;

    /**
     * @param key Long key of the command
     */
    virtual void setLongKey(const std::string &key) = 0;

    /**
     * @param isLocal Indicates if command should be executed locally or sent
     * for execution (on external server)
     */
    virtual void setLocal(bool isLocal) = 0;

    /**
     * @retval True if command is local (can be executed locally)
     * @retval False if command is not local (should be sent
     * for execution on external server)
     */
    virtual bool isLocal() const = 0;

    /**
     * @brief Execute command locally (in the same process)
     *
     * @note Commands on client side which are the request for external
     * server do not have this method implemented. In such case IComannd
     * serves as a container of parameters for command and should be
     * sent and executed on server by external object.
     */
    virtual void execute() = 0;

    /**
     * @param desc Description of the command
     */
    virtual void setDesc(const std::string &desc) = 0;

    /**
     * @brief Add parameter to this command
     *
     * @param param Parameter to be added
     */
    virtual void addParam(std::shared_ptr<IParameter> param) = 0;

    /**
     * @return Is this command hidden (doesn't show up in help)
     */
    virtual bool isHidden() const = 0;

    /**
     * @return Sets if the command is hidden (doesn't show up in help)
     */
    virtual void setHidden(bool hidden) = 0;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CMD_ICOMMAND_H
