/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_EXECUTOR_H
#define SOURCE_OCTF_CLI_EXECUTOR_H

#include <initializer_list>
#include <map>
#include <octf/cli/CLIList.h>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/CLIUtils.h>
#include <octf/cli/CommandSet.h>
#include <octf/cli/GenericPluginShadow.h>
#include <octf/cli/Module.h>
#include <octf/cli/cmd/CmdHelp.h>
#include <octf/cli/cmd/CommandProtobuf.h>
#include <octf/node/INode.h>
#include <octf/utils/Log.h>

namespace octf {

// This value specifies delimiter for multiple value parameters
constexpr char PARAMETER_VALUE_DELIMITER[] = ",";

class Parameter;

/**
 * @brief Class used for command execution
 */
class Executor {
public:
    /**
     * @brief Default constructor
     */
    Executor();
    virtual ~Executor() = default;

    /**
     * @brief Gets CLI Properties
     *
     * @return CLI Properties to be set
     */
    CLIProperties &getCliProperties();

    /**
     * @brief Adds interface into this CLI executor
     *
     * @param interfaces Interfaces initializer list
     */
    void addInterfaces(std::initializer_list<InterfaceShRef> interfaces);

    /**
     * @brief Execute command
     *
     * @param argc Arguments count
     * @param argv Arguments array
     *
     * @return Execution result
     * @retval 0 Success
     * @retval Non-zero Failure
     */
    int execute(int argc, char *argv[]);

private:
    /**
     * @brief Add local module with key prefix
     *
     * @param interface Interface which method will be added into local module
     * @param longKey Module long key
     * @param desc Module description
     * @param shortKey Module short key
     */
    void addLocalModule(InterfaceShRef interface,
                        const std::string &longKey,
                        const std::string &desc,
                        const std::string &shortKey = "");

    /**
     * @brief Add local module
     *
     * @param interface Interface which methods will be add into local command
     * set
     */
    void addLocalModule(InterfaceShRef interface);

    /**
     * @brief Add local command to executor's command set
     * @param cmd Command to be added
     */
    void addLocalCommand(std::shared_ptr<ICommand> cmd);

    /**
     * @brief Execute command
     * @param cliList CLIList with parameters
     *
     * @return Execution result
     * @retval 0 Success
     * @retval Non-zero Failure
     */
    int execute(CLIList &cliList);

    void addInterface(InterfaceShRef interface, CommandSet &commandSet);

    /**
     * @brief Add a proto method which will be available as a local command
     *
     * @param method Pointer to method descriptor
     * @param interface Pointer to interface containing given method
     * @param commandSet Command set to contain this command
     */
    void addMethod(const ::google::protobuf::MethodDescriptor *method,
                   InterfaceShRef interface,
                   CommandSet &commandSet);

    /**
     * @return Help for executor's local commands and available plugins
     */
    void printMainHelp(std::stringstream &ss);

    /**
     * @brief Get internally a set of available modules
     */
    void getModules();

    /**
     * @brief Set execution progress and print if changed
     * @param progress Progress of command as a fraction
     * @param out Output stream
     */
    void setProgress(double progress, std::ostream &out);

    /**
     * @brief Load command set of given (remote) module
     *
     * This method may need to communicate with specified module
     * in order to load command set
     */
    void loadModuleCommandSet();

    std::shared_ptr<ICommand> getCommandFromModule(std::string cmdName);

    bool isModuleExistent(std::string moduleName) const;

    void setModule(std::string moduleName);

    std::shared_ptr<ICommand> validateCommand(CLIList &cliList);

    void executeRemote(std::shared_ptr<CommandProtobuf> cmd);

    /**
     * Prepares output to handle commands' logs
     *
     * When parameters validation phase ended, and there are no more help
     * messages, we can adjust outputs to print commands' logs
     */
    void setupOutputsForCommandsLogs() const;

private:
    CLIProperties m_cliProperties;
    CommandSet m_localCmdSet;
    CommandSet m_moduleCmdSet;
    std::map<std::string, Module> m_modules;
    Module m_module;
    double m_progress;
    std::unique_ptr<GenericPluginShadow> m_nodePlugin;
    std::map<std::string, CommandSet> m_localModules;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_EXECUTOR_H
