/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cassert>
#include <exception>
#include <iomanip>
#include <sstream>
#include <string>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/cli/internal/CommandSet.h>
#include <octf/cli/internal/cmd/CmdHelp.h>
#include <octf/cli/internal/cmd/ICommand.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>

extern "C" {
#include "errno.h"
}

using namespace std;

namespace octf {
namespace cli {

CommandSet::CommandSet()
        : m_cmds()
        , m_iter(m_cmds.begin()) {
    // Add default help command
    m_helpCmd = make_shared<CmdHelp>();
    addCmd(m_helpCmd);
    updateHelp();
}

shared_ptr<ICommand> CommandSet::getCmd(string cmd) {
    map<string, shared_ptr<ICommand>>::iterator iter = m_cmds.begin();

    for (; iter != m_cmds.end(); iter++) {
        if (iter->second->getLongKey() == cmd ||
            iter->second->getShortKey() == cmd) {
            return iter->second;
        }
    }

    throw InvalidParameterException("Unknown command.");
}

void CommandSet::getHelp(stringstream &ss) const {
    map<string, shared_ptr<ICommand>>::const_iterator iter = m_cmds.begin();
    for (; iter != m_cmds.end(); iter++) {
        string shortKey = iter->second->getShortKey();
        string longKey = iter->second->getLongKey();
        string desc = iter->second->getDesc();

        utils::printKeys(ss, shortKey, longKey, desc);
    }
}

void CommandSet::addCmd(std::shared_ptr<ICommand> cmd) {
    if ("" == cmd->getLongKey()) {
        throw InvalidParameterException("Added command has no long key.");
    }

    const auto &shortKey = cmd->getShortKey();
    if (!shortKey.empty() && hasCmd(shortKey)) {
        throw Exception("Cannot add command because of short key conflict: " +
                        shortKey);
    }

    const auto &longKey = cmd->getLongKey();
    if (hasCmd(longKey)) {
        throw Exception("Cannot add command because of long key conflict: " +
                        longKey);
    }

    m_cmds[cmd->getLongKey()] = cmd;
}

void CommandSet::clear() {
    m_cmds.clear();
    m_iter = m_cmds.begin();
}

bool CommandSet::hasCmd(const std::string &cmdName) const {
    map<string, shared_ptr<ICommand>>::const_iterator iter = m_cmds.begin();

    for (; iter != m_cmds.end(); iter++) {
        if (iter->second->getLongKey() == cmdName ||
            iter->second->getShortKey() == cmdName) {
            return true;
        }
    }

    return false;
}

void CommandSet::setHelpCmdContent(const std::string &help) {
    m_helpCmd->setHelp(help);
}

void CommandSet::updateHelp() {
    stringstream ss;
    map<string, shared_ptr<ICommand>>::const_iterator iter = m_cmds.begin();
    for (; iter != m_cmds.end(); iter++) {
        string shortKey = iter->second->getShortKey();
        string longKey = iter->second->getLongKey();
        string desc = iter->second->getDesc();

        utils::printKeys(ss, shortKey, longKey, desc);
    }
    m_helpCmd->setHelp(ss.str());
}

std::shared_ptr<CmdHelp> CommandSet::getHelpCmd() const {
    return m_helpCmd;
}

}  // namespace cli
}  // namespace octf
