/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <cassert>
#include <exception>
#include <iomanip>
#include <map>
#include <sstream>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/cli/internal/cmd/Command.h>
#include <octf/cli/internal/param/ParamHelp.h>
#include <octf/utils/Exception.h>

extern "C" {
#include "errno.h"
}

using namespace std;

namespace octf {
namespace cli {

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

    throw Exception("Option '" + name + "' is unrecognized.");
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
    // Look for command parameters
    while (cliList.hasNext()) {
        // Get next parameter

        // Get next element - a key is expected
        CLIElement element = cliList.nextElement();
        string name = element.getValidKeyName();
        if (name.empty()) {
            throw Exception("Invalid parameter format");
        }

        // Find proper parameter
        std::shared_ptr<IParameter> param = getParam(name);

        // Parse input for given parameter
        if (param->hasValue()) {
            if (cliList.hasNext()) {
                // Get next element - a value (not a key) is expected
                param->setValue(cliList.next());
            } else {
                throw Exception("Value of option '" + param->getLongKey() +
                                "' is missing.");
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
        utils::printKeys(ss, shortKey, longKey, desc);
    }
}

void Command::getCommandUsage(stringstream &ss) const {
    bool optionalParams = false;

    utils::printKeys(ss, "", getLongKey(), "", false);

    // Print all required options
    auto iter = m_params.begin();
    for (; iter != m_params.end(); iter++) {
        if (false == iter->second->isRequired()) {
            optionalParams = true;
            continue;
        }
        string longKey = iter->second->getLongKey();
        string desc = iter->second->getWhat();
        utils::printKeys(ss, "", longKey, desc, false);
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
        throw Exception("Option '" + iter->second->getLongKey() +
                        "' is required.");
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

}  // namespace cli
}  // namespace octf
