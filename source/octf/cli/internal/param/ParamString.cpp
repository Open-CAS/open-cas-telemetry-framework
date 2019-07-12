/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/message.h>
#include <cstdio>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/param/ParamString.h>
#include <octf/utils/Exception.h>

extern "C" {
#include <errno.h>
}

using namespace std;

namespace octf {
namespace cli {

ParamString::ParamString()
        : Parameter()
        , m_values()
        , m_defaultValue(1, "")
        , m_maxLength(MAX_STRING_LENGTH)
        , m_multipleValue(false) {}

void ParamString::setValue(CLIElement element) {
    if (!isValueSet()) {
        string inputValue = element.getValue();

        if (inputValue.size() > m_maxLength) {
            throw InvalidParameterException("Value length of option '" +
                                            getLongKey() + "' is too long.");
        }

        // Parse input for delimited multiple values
        m_values = parseValuesToVector(inputValue);
        setValueSet();

    } else {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is set multiple times.");
    }
}

const std::vector<std::string> &ParamString::getValue() const {
    if (!isValueSet()) {
        return m_defaultValue;
    } else {
        return m_values;
    }
}

void ParamString::setValue(std::string value) {
    m_values = parseValuesToVector(value);
    setValueSet();
}

bool ParamString::isValid(const std::string &value) {
    if (value.size() > MAX_STRING_LENGTH) {
        return false;
    } else {
        return true;
    }
}

uint32_t ParamString::getMaxLength() const {
    return m_maxLength;
}

void ParamString::setMaxLength(uint32_t maxLength) {
    m_maxLength = maxLength;
}

void ParamString::setDefault(std::string values) {
    // Parse delimited values to vector
    m_defaultValue = parseValuesToVector(values);
}

void ParamString::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::FieldDescriptor *fieldDescriptor) {
    if (fieldDescriptor->is_repeated()) {
        for (unsigned int i = 0; i < m_values.size(); i++) {
            message->GetReflection()->AddString(message, fieldDescriptor,
                                                m_values[i]);
        }
    } else if (m_values.size()) {
        message->GetReflection()->SetString(message, fieldDescriptor,
                                            m_values[0]);
    }
}

void ParamString::setOptions(
        const google::protobuf::FieldDescriptor *fieldDesc) {
    const google::protobuf::FieldOptions &fieldOptions = fieldDesc->options();

    if (!fieldOptions.HasExtension(proto::opts_param)) {
        throw Exception("Input message's field: " + fieldDesc->name() +
                        " does not have parameter options");
    }
    const proto::OptsParam &paramOps =
            fieldOptions.GetExtension(proto::opts_param);

    // Set options independent of type
    Parameter::setOptions(fieldDesc);

    // If string-specific options are present, set them
    if (paramOps.has_cli_str()) {
        setDefault(paramOps.cli_str().default_value());
    }
}

void ParamString::setMultipleValue(bool multipleValue) {
    m_multipleValue = multipleValue;

    if (multipleValue) {
        setWhat("<VALUE>[,VALUE]");
    }
}

bool ParamString::isMultipleValue() const {
    return m_multipleValue;
}

std::vector<std::string> ParamString::parseValuesToVector(
        std::string values) const {
    size_t pos = 0;
    std::string token;
    std::vector<std::string> result;
    constexpr int DELIMITER_LENGTH = sizeof(PARAMETER_VALUE_DELIMITER) - 1;

    // Parse input for delimited multiple values
    // TODO (trybicki): Implement escaping of delimiter sign
    while ((pos = values.find(PARAMETER_VALUE_DELIMITER)) !=
           std::string::npos) {
        token = values.substr(0, pos);
        result.push_back(token);
        values.erase(0, pos + DELIMITER_LENGTH);
    }
    result.push_back(values);

    return result;
}

}  // namespace cli
}  // namespace octf
