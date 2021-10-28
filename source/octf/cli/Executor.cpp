/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/cli/Executor.h>

#include <google/protobuf/dynamic_message.h>
#include <algorithm>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/cli/internal/CommandSet.h>
#include <octf/cli/internal/GenericPluginShadow.h>
#include <octf/cli/internal/Module.h>
#include <octf/cli/internal/OptionsValidation.h>
#include <octf/cli/internal/cmd/CmdHelp.h>
#include <octf/cli/internal/cmd/CmdVersion.h>
#include <octf/cli/internal/cmd/CommandProtobuf.h>
#include <octf/cli/internal/cmd/CommandProtobufLocal.h>
#include <octf/proto/opts.pb.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/ModulesDiscover.h>

using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::vector;

namespace octf {
namespace cli {

Executor::Executor()
        : m_cliProperties()
        , m_localCmdSet(std::make_shared<CommandSet>())
        , m_cmdSet(nullptr)
        , m_module(new Module())
        , m_progress(0.0)
        , m_nodePlugin(nullptr)
        , m_modules()
        , m_supportedRemoteModules() {
    addLocalCommand(make_shared<CmdVersion>(m_cliProperties));
}

Executor::~Executor() {}

CLIProperties &Executor::getCliProperties() {
    return m_cliProperties;
}

void Executor::addLocalCommand(shared_ptr<ICommand> cmd) {
    m_localCmdSet->addCmd(cmd);
}

void Executor::loadModuleCommandSet() {
    if (!m_module->isLocal()) {
        // Get description of this module's command set
        Call<proto::Void, proto::CliCommandSetDesc> call(m_nodePlugin.get());
        m_nodePlugin->getCliInterface()->getCliCommandSetDescription(
                &call, call.getInput().get(), call.getOutput().get(), &call);
        call.wait();

        if (call.Failed()) {
            throw InvalidParameterException("Cannot get command list, error: " +
                                            call.ErrorText());
        }

        auto cmdSetDesc = call.getOutput();
        if (utils::isCommandSetValid(*cmdSetDesc)) {
            int commandCount = cmdSetDesc->command_size();
            for (int i = 0; i < commandCount; i++) {
                const proto::CliCommandDesc &cmdDesc = cmdSetDesc->command(i);
                auto cmd = make_shared<CommandProtobuf>(cmdDesc);
                m_cmdSet->addCmd(cmd);
            }
        }
    }
}

void Executor::printMainHelp(std::stringstream &ss) {
    utils::printUsage(ss, nullptr, m_cliProperties, false, m_modules.size());

    if (m_modules.size()) {
        ss << endl << "Available modules: " << endl;
        for (const auto iter : m_modules) {
            utils::printModuleHelp(ss, &iter.first, true);
        }
    }

    ss << std::endl << "Available commands: " << std::endl;
    utils::printCmdSetHelp(ss, *m_localCmdSet);
}

void Executor::printModuleHelp(std::stringstream &ss) {
    // Initialize module command set if needed
    loadModuleCommandSet();

    utils::printUsage(ss, m_module.get(), m_cliProperties, false);
    if (m_module->getDesc() != "") {
        ss << "\n" << m_module->getDesc() << std::endl;
    }
    ss << std::endl << "Available commands: " << std::endl;
    utils::printCmdSetHelp(ss, *m_cmdSet);
}

void Executor::discoverModules() {
    bool found = false;

    // No remote modules supported anyway, so quit
    if (m_supportedRemoteModules.empty()) {
        return;
    }

    ModulesDiscover discover;
    NodesIdList nodes;

    // Get a list of modules which sockets were detected
    discover.getModulesList(nodes);

    // Add only modules which are explicitly supported
    for (const NodeId &node : nodes) {
        NodesIdList::iterator result;
        result = std::find(m_supportedRemoteModules.begin(),
                           m_supportedRemoteModules.end(), node);

        if (result != m_supportedRemoteModules.end()) {
            Module newModule;
            newModule.setLongKey(node.getId());

            auto entry = std::make_pair(newModule, CommandSet());
            m_modules[newModule] = std::make_shared<CommandSet>();
            found = true;
        }
    }

    if (!found) {
        m_supportedRemoteModules.clear();
    }
}

shared_ptr<ICommand> Executor::lookupCommand(CLIList &cliList,
                                             std::string &errMsg) {
    CLIElement element = cliList.nextElement();
    string key = element.getValidKeyName();
    if (key.empty()) {
        throw InvalidParameterException("Invalid command format.");
    }
    string cmd;

    // Set initial command set to local one
    m_cmdSet = m_localCmdSet;
    m_module->setLocal(true);

    if (lookupModule(key)) {
        // Dealing with module, the next parameter is command switch

        if (cliList.hasNext()) {
            element = cliList.nextElement();
            cmd = element.getValidKeyName();
            if (cmd.empty()) {
                errMsg = "Invalid command format.";
                return nullptr;
            }
        } else {
            errMsg = "No command specified";
            return nullptr;
        }
    } else {
        // Dealing with local command
        cmd = key;
    }

    // Look for command in local or module command set
    shared_ptr<ICommand> commandToExecute;
    if (m_module->isLocal()) {
        // Local command
        commandToExecute = m_cmdSet->getCmd(cmd);
    } else {
        // Module command
        if (m_cmdSet->hasCmd(cmd)) {
            // Module command set already loaded
            commandToExecute = m_cmdSet->getCmd(cmd);
        } else {
            // Module command set not loaded or command not existent
            commandToExecute = getCommandFromModule(cmd);
        }
    }

    return commandToExecute;
}

shared_ptr<ICommand> Executor::getCommandFromModule(string cmdName) {
    if (!m_nodePlugin.get()) {
        return nullptr;
    }
    Call<proto::CliCommandId, proto::CliCommandDesc> call(m_nodePlugin.get());
    call.getInput()->set_commandkey(cmdName);

    m_nodePlugin->getCliInterface()->getCliCommandDescription(
            &call, call.getInput().get(), call.getOutput().get(), &call);
    call.wait();

    if (call.Failed()) {
        throw InvalidParameterException(
                "Cannot get command description, error: " + call.ErrorText());
    }

    const proto::CliCommandDesc &cliCmd = *call.getOutput();
    if (utils::isCommandValid(cliCmd)) {
        return make_shared<CommandProtobuf>(cliCmd);
    } else {
        return nullptr;
    }
}

int Executor::execute(CLIList &cliList) {
    // Find supported and available modules by discovering sockets
    discoverModules();

    std::string errMsg = "";
    shared_ptr<ICommand> command = lookupCommand(cliList, errMsg);
    bool printHelp = command == nullptr || command == m_cmdSet->getHelpCmd();

    if (printHelp) {
        if (!errMsg.empty()) {
            log::cerr << errMsg << std::endl;
        }

        if (m_cmdSet == m_localCmdSet) {
            // "First level" help (general for application)
            stringstream ss;
            printMainHelp(ss);
            log::cout << ss.str();
        } else {
            stringstream ss;
            printModuleHelp(ss);
            log::cout << ss.str();
        }

        return command == nullptr;
    }

    try {
        if (cliList.hasHelp()) {
            // Help requested, print it and return
            stringstream ss;
            utils::printCmdHelp(ss, command, m_cliProperties);
            log::cout << ss.str();
            return 0;
        }

        // Fill command's parameters
        command->parseParamValues(cliList);

    } catch (Exception &e) {
        // An error during parsing command, print it and then print help,
        // and return

        if ("" != e.getMessage()) {
            log::cerr << e.getMessage() << endl;
        }

        stringstream ss;
        utils::printCmdHelp(ss, command, m_cliProperties);
        log::cout << ss.str();

        return 1;
    }

    setupOutputsForCommandsLogs();

    if (command->isLocal()) {
        // Execute command locally
        command->execute();
    } else {
        // Execute remotely
        shared_ptr<CommandProtobuf> protoCmd =
                std::dynamic_pointer_cast<CommandProtobuf>(command);
        if (protoCmd) {
            executeRemote(protoCmd);

        } else {
            // Dynamic cast failed
            throw InvalidParameterException("Unknown command type.");
        }
    }

    return 0;
}

int Executor::execute(int argc, char *argv[]) {
    int result = 1;

    if (argc > 1) {
        // Parse application input
        vector<string> arguments(argv, argv + argc);
        CLIList cliList;
        cliList.create(arguments);

        // Execute command
        result = execute(cliList);

    } else {
        throw InvalidParameterException(
                "Specify module or command first. Use '" +
                m_cliProperties.getName() + " -H' for help.");
    }

    return result;
}

bool Executor::lookupModule(const std::string &name) {
    for (const auto iter : m_modules) {
        const auto &module = iter.first;
        if (name == module.getShortKey() || name == module.getLongKey()) {
            *m_module = module;
            m_cmdSet = iter.second;
            break;
        }
    }

    if ("" == m_module->getLongKey()) {
        // Module not found
        return false;
    }

    if (!m_module->isLocal()) {
        // Initialize node plug-in if module is remote
        m_nodePlugin.reset(new GenericPluginShadow(m_module->getLongKey()));
        if (!m_nodePlugin->init()) {
            throw Exception("Module unavailable.");
        }
    }

    return true;
}

void Executor::addInterface(InterfaceShRef interface, CommandSet &commandSet) {
    // Add every method of interface as command to given command set
    int methodCount = interface->GetDescriptor()->method_count();
    for (int i = 0; i < methodCount; i++) {
        addMethod(interface->GetDescriptor()->method(i), interface, commandSet);
    }
}

void Executor::addMethod(const ::google::protobuf::MethodDescriptor *method,
                         InterfaceShRef interface,
                         CommandSet &commandSet) {
    if (&commandSet == m_localCmdSet.get()) {
        // This method is added to local module set (to gloabl scope), check if
        // there is no long and short key conflict among local command set and
        // local modules.

        const auto &mOpts = method->options().GetExtension(proto::opts_command);
        const auto &shortKey = mOpts.cli_short_key();
        const auto &LongKey = mOpts.cli_long_key();
        checkIsKeyRepeated(shortKey, LongKey);
    }

    // TODO(trybicki): Don't create commands upfront for every interface
    // method Create local command and add it to command set
    std::shared_ptr<CommandProtobufLocal> cmd =
            make_shared<CommandProtobufLocal>(method, interface);
    commandSet.addCmd(cmd);
}

void Executor::addLocalModule(InterfaceShRef interface,
                              const std::string &longKey,
                              const std::string &desc,
                              const std::string &shortKey) {
    checkIsKeyRepeated(shortKey, longKey);

    Module module;
    module.setDesc(desc);
    module.setLongKey(longKey);
    module.setShortKey(shortKey);
    module.setLocal(true);

    // Create command set for interface
    auto cmdSet = std::make_shared<CommandSet>();
    m_modules[module] = cmdSet;

    // Add interface to the command set
    addInterface(interface, *cmdSet);
}

void Executor::addModule(InterfaceShRef interface) {
    auto iDesc = interface->GetDescriptor();
    const auto &iOpts = iDesc->options().GetExtension(proto::opts_interface);

    if ("" != iOpts.cli_long_key()) {
        addLocalModule(interface, iOpts.cli_long_key(), iOpts.cli_desc(),
                       iOpts.cli_short_key());
    } else {
        addInterface(interface, *m_localCmdSet);
    }
}

void Executor::addModule(const NodeId &moduleId) {
    NodesIdList::iterator result;
    result = std::find(m_supportedRemoteModules.begin(),
                       m_supportedRemoteModules.end(), moduleId);

    // Add only non-duplicate NodeIds
    if (result == m_supportedRemoteModules.end()) {
        m_supportedRemoteModules.push_back(moduleId);
    } else {
        throw Exception("Duplicate remote module ids added");
    }
}

void Executor::executeRemote(std::shared_ptr<CommandProtobuf> cmd) {
    if (!m_nodePlugin) {
        throw Exception("Wrong initialization of plugin.");
    }
    // Factory to create protobuf messages in runtime
    google::protobuf::DynamicMessageFactory factory;

    // Create prototype of input message based on command input descriptor
    // Factory keeps ownership of memory
    const google::protobuf::Message *prototype_msg =
            factory.GetPrototype(cmd->getInputDesc());

    // Create mutable input message from prototype
    // Ownership passed to the caller, thus use smart pointer to handle it
    MessageShRef in_msg = MessageShRef(prototype_msg->New());

    // Parse parameters - set values stored in command to the protobuf
    // message
    cmd->parseToProtobuf(in_msg.get(), cmd->getInputDesc());

    // Create prototype of output message based on command output descriptor
    // Factory keeps ownership of memory
    const google::protobuf::Message *prototype_out =
            factory.GetPrototype(cmd->getOutputDesc());

    // Create mutable output message from prototype
    // Ownership passed to the caller, thus use smart pointer to handle it
    MessageShRef out_msg = MessageShRef(prototype_out->New());

    CallGeneric call(in_msg, out_msg, m_nodePlugin.get());
    // Remote method call
    m_nodePlugin->getRpcChannel()->genericCallMethod(cmd->getInterfaceId(),
                                                     cmd->getMethodId(), call);

    // Wait for result and print output
    cmd->handleCall(call, out_msg);
}

void Executor::setProgress(double progress, std::ostream &out) {
    uint64_t next = progress * 100;
    uint64_t prev = m_progress * 100;

    if (next != prev) {
        m_progress = progress;
        utils::printProgressBar(m_progress, out);
    }
}

void Executor::setupOutputsForCommandsLogs() const {
    auto const &prefix = m_cliProperties.getName();

    if (nullptr != getenv("VERBOSE")) {
        log::verbose << log::enable << log::json << log::prefix << prefix;
        log::debug << log::enable << log::json << log::prefix << prefix;
    } else {
        log::verbose << log::disable;
        log::debug << log::disable;
    }

    log::cerr << log::enable << log::json << log::prefix << prefix;
    log::critical << log::enable << log::json << log::prefix << prefix;
    log::cout << log::enable << log::json << log::prefix << prefix;
}

void Executor::checkIsKeyRepeated(const std::string &shortKey,
                                  const std::string &longKey) const {
    if (m_localCmdSet->hasCmd(longKey)) {
        throw Exception("Cannot add command because of long key conflict: " +
                        longKey);
    }

    if (!shortKey.empty() && m_localCmdSet->hasCmd(shortKey)) {
        throw Exception("Cannot add command because of short key conflict: " +
                        shortKey);
    }

    for (const auto iter : m_modules) {
        if (iter.first.getLongKey() == longKey) {
            throw Exception(
                    "Cannot add command because of long key conflict: " +
                    longKey);
        }

        if (!shortKey.empty() && iter.first.getShortKey() == shortKey) {
            throw Exception(
                    "Cannot add command because of short key conflict: " +
                    shortKey);
        }
    }
}

}  // namespace cli
}  // namespace octf
