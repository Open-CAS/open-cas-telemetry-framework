/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <cassert>
#include <exception>
#include <iomanip>
#include <map>
#include <sstream>
#include <octf/cli/CLIException.h>
#include <octf/cli/CLIList.h>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/CLIUtils.h>
#include <octf/cli/cmd/Command.h>
#include <octf/cli/param/ParamHelp.h>
#include <octf/utils/Exception.h>

extern "C" {
#include "errno.h"
}

using namespace std;

namespace octf {

Command::Command()
        : m_shortKey("")
        , m_longKey("")
        , m_desc("")
        , m_params()
        , m_hidden(false)
        , m_local(true) {}

Command::~Command() {
    m_params.clear();
}

void Command::setShortKey(const std::string &key) {
    m_shortKey = key;
}

void Command::setLongKey(const std::string &key) {
    m_longKey = key;
}

bool Command::isHidden() const {
    return m_hidden;
}

void Command::setHidden(bool hidden) {
    m_hidden = hidden;
}

void Command::addParam(std::shared_ptr<IParameter> param) {
    if (m_params.find(param->getLongKey()) != m_params.end()) {
        throw InvalidParameterException("Parameter " + param->getLongKey() +
                                        " added twice.");
    }

    m_params[param->getLongKey()] = param;
}

std::shared_ptr<IParameter> Command::getParam(const string &name) {
    map<string, shared_ptr<IParameter>>::iterator iter = m_params.begin();

    for (; iter != m_params.end(); iter++) {
        if ((iter->second)->getShortKey() == name ||
            (iter->second)->getLongKey() == name) {
            return iter->second;
        }
    }

    throw CLIException("Option '" + name + "' is unrecognized.", true, true);
}

std::shared_ptr<IParameter> Command::getParamByIndex(const int32_t index) {
    map<string, shared_ptr<IParameter>>::iterator iter = m_params.begin();

    for (; iter != m_params.end(); iter++) {
        if ((iter->second)->getIndex() == index) {
            return iter->second;
        }
    }

    throw InvalidParameterException("Option index '" + std::to_string(index) +
                                    "' is unrecognized.");
}

void Command::parseParamValues(CLIList &cliList) {
    ParamHelp ph;
    // Look for command parameters
    while (cliList.hasNext()) {
        // Get next parameter

        // Get next element - a key is expected
        CLIElement element = cliList.nextElement();
        string name = element.getValidKeyName();
        if (name.empty()) {
            throw CLIException("Invalid parameter format", true, true);
        }

        // Show third level help if the parameter is help
        if (name == ph.getLongKey() || name == ph.getShortKey()) {
            // Throw exception in order to not execute command and show help
            throw CLIException("", true, false);
        }

        // Find proper parameter
        std::shared_ptr<IParameter> param = getParam(name);

        // Parse input for given parameter
        if (param->hasValue()) {
            if (cliList.hasNext()) {
                // Get next element - a value (not a key) is expected
                param->setValue(cliList.next());
            } else {
                throw CLIException("Value of option '" + param->getLongKey() +
                                           "' is missing.",
                                   true, true);
            }

        } else {
            // Set flag parameter value to empty
            param->setValue(CLIElement(""));
        }
    }

    if (cliList.hasNext()) {
        throw InvalidParameterException("Too much option(s).");
    }

    // if parameter is missing
    checkParamMissing();
}

void Command::getHelp(stringstream &ss) const {
    auto iter = m_params.begin();
    for (; iter != m_params.end(); iter++) {
        string shortKey = iter->second->getShortKey();
        string longKey = iter->second->getLongKey();
        string desc = iter->second->getDesc();
        string what = iter->second->getWhat();

        if ("" != what) {
            longKey += (" " + what);
        }
        CLIUtils::printKeys(ss, shortKey, longKey, desc);
    }
}

void Command::getCommandUsage(stringstream &ss) const {
    bool optionalParams = false;

    CLIUtils::printKeys(ss, "", getLongKey(), "", false);

    // Print all required options
    auto iter = m_params.begin();
    for (; iter != m_params.end(); iter++) {
        if (false == iter->second->isRequired()) {
            optionalParams = true;
            continue;
        }
        string longKey = iter->second->getLongKey();
        string desc = iter->second->getWhat();
        CLIUtils::printKeys(ss, "", longKey, desc, false);
    }

    // If there are any optional (not required) parameters, add general usage
    // field
    if (optionalParams) {
        ss << " [options...]";
    }
}

void Command::checkParamMissing() const {
    // Iterate over command's parameters
    auto iter = m_params.begin();
    for (; iter != m_params.end(); iter++) {
        if (iter->second->isRequired() == false) {
            // Not required parameters can be missing
            continue;
        }

        if (iter->second->isValueSet()) {
            // If parameter is set then it's not missing
            continue;
        }

        // There is a required and not set parameter
        throw CLIException(
                "Option '" + iter->second->getLongKey() + "' is required.",
                true, true);
    }

    // All required parameters are set
}

const string &Command::getShortKey() const {
    return m_shortKey;
}

const string &Command::getLongKey() const {
    return m_longKey;
}

const string &Command::getDesc() const {
    return m_desc;
}

uint32_t Command::getParamsCount() const {
    return m_params.size();
}

void Command::setDesc(const string &desc) {
    m_desc = desc;
}

bool Command::isLocal() const {
    return m_local;
}

void Command::setLocal(bool local) {
    m_local = local;
}

}  // namespace octf
