/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <cassert>
#include <cstdint>
#include <exception>
#include <map>
#include <sstream>
#include <string>
#include <octf/cli/CLIList.h>
#include <octf/cli/param/ParamEnum.h>
#include <octf/cli/param/Parameter.h>
#include <octf/utils/Exception.h>

using namespace std;

namespace octf {

ParamEnum::ParamEnum()
        : Parameter()
        , m_enums()
        , m_value()
        , m_default(0)
        , m_hasDefault(false) {}

void ParamEnum::setValue(CLIElement element) {
    bool found = false;

    map<int64_t, Item>::const_iterator iter = m_enums.begin();
    for (; iter != m_enums.end(); iter++) {
        if (element.getValue() == iter->second.getName()) {
            found = true;
            m_value = iter->second.getValue();
        }
    }

    if (false == found) {
        throw InvalidParameterException("Value " + element.getValue() +
                                        " of option '" + getLongKey() +
                                        "' is invalid.");
    }

    setValueSet();
}

bool ParamEnum::hasEnum(int64_t value, const string &name) const {
    if (m_enums.end() == m_enums.find(value)) {
        return false;
    }

    map<int64_t, Item>::const_iterator iter = m_enums.begin();
    for (; iter != m_enums.end(); iter++) {
        if (name == iter->second.getName() &&
            value == iter->second.getValue()) {
            return true;
        }
    }

    return false;
}

string ParamEnum::getDescShort() const {
    string shrt = "{";

    map<int64_t, Item>::const_iterator iter = m_enums.begin();
    for (; iter != m_enums.end(); iter++) {
        if (iter != m_enums.begin()) {
            shrt += "|";
        }

        shrt += iter->second.getName();
    }

    shrt += "}";

    return shrt;
}

string ParamEnum::getDescLong() const {
    string lng = "";

    map<int64_t, Item>::const_iterator iter = m_enums.begin();
    for (; iter != m_enums.end(); iter++) {
        if (iter != m_enums.begin()) {
            lng += ", ";
        }

        lng += iter->second.getName();
        if ("" != iter->second.getDesc()) {
            lng += " - " + iter->second.getDesc();
        }

        if (hasDefault()) {
            if (m_default == iter->first) {
                lng += " (default)";
            }
        }
    }

    return lng;
}

void ParamEnum::details() {
    m_details = Parameter::getDesc();
    m_details += ", " + getDescLong();
    setWhat(getDescShort());
}

int64_t ParamEnum::stringToValue(const string &name) const {
    map<int64_t, Item>::const_iterator iter = m_enums.begin();
    for (; iter != m_enums.end(); iter++) {
        if (name == iter->second.getName()) {
            return iter->second.getValue();
        }
    }

    throw InvalidParameterException("Value of option '" + getLongKey() +
                                    "' is invalid.");
}

void ParamEnum::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::FieldDescriptor *fieldDescriptor) {
    message->GetReflection()->SetEnumValue(message, fieldDescriptor, m_value);
}

void ParamEnum::addEnumVal(int64_t value,
                           const string &name,
                           const string &desc) {
    if (hasEnum(value, name)) {
        assert(false);
        return;
    }

    Item item(value, name, desc);
    m_enums[value] = item;

    details();
}

void ParamEnum::setDefault(int64_t _default) {
    if (m_enums.end() == m_enums.find(_default)) {
        assert(false);
    }

    m_default = _default;
    m_value = _default;
    m_hasDefault = true;
    details();
}

string ParamEnum::getStringValue(int64_t value) const {
    map<int64_t, Item>::const_iterator iter = m_enums.find(value);
    if (m_enums.end() == iter) {
        return "Unknown value";
    }

    string _value = iter->second.getDesc();
    if ("" != _value) {
        _value += (" (" + iter->second.getName() + ")");
    } else {
        _value = iter->second.getName();
    }

    return _value;
}

string ParamEnum::valueToString(int64_t val) const {
    map<int64_t, Item>::const_iterator iter = m_enums.find(val);

    if (iter == m_enums.end()) {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is invalid.");
    }

    return iter->second.getName();
}

int64_t ParamEnum::getValue() const {
    return m_value;
}

int64_t ParamEnum::getDefault() const {
    return m_default;
}

bool ParamEnum::hasDefault() const {
    return m_hasDefault;
}

const std::string &ParamEnum::getDesc() const {
    return m_details;
}

void ParamEnum::setDesc(const std::string &desc) {
    Parameter::setDesc(desc);
    details();
}

void ParamEnum::setOptions(const proto::CliParameter &paramDef) {
    const proto::OptsParam &paramOps = paramDef.paramops();

    // Set options independent of type
    Parameter::setOptions(paramDef);

    // If enum-specific options are present, set them
    if (paramOps.has_cli_enum()) {
        int enumCount = paramOps.cli_enum().enum_value_size();

        for (int i = 0; i < enumCount; i++) {
            const auto &enumValue = paramOps.cli_enum().enum_value(i);

            // Add possible values of enum to enum parameter
            // TODO(trybicki): Validate enum switch
            addEnumVal(enumValue.value(), enumValue.cli_switch(),
                       enumValue.cli_desc());
        }

        // Set default value if parameter is not required
        if (!paramOps.cli_required()) {
            setDefault(paramOps.cli_enum().default_value());
        }
    } else {
        throw Exception(
                "Enum: " + std::string(paramOps.cli_long_key()) +
                " defined in .proto file doesn't contain cli_enum options");
    }
}

}  // namespace octf
