/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
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
        , m_defaultValues(1, "")
        , m_hasDefaultValue(false)
        , m_maxLength(MAX_STRING_LENGTH)
        , m_multipleValue(false)
        , m_multipleValueLimit(0) {}

void ParamString::setValue(CLIElement element) {
    if (!isValueSet()) {
        const auto &inputValue = element.getValue();

        if (inputValue.size() > m_maxLength) {
            throw InvalidParameterException("Value length of option '" +
                                            getLongKey() + "' is too long.");
        }

        setValue(inputValue);
    } else {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is set multiple times.");
    }
}

const std::string &ParamString::getValue() const {
    if (!isMultipleValue()) {
        if (isValueSet()) {
            return m_values[0];
        } else {
            return m_defaultValues[0];
        }
    } else {
        throw InvalidParameterException("Option '" + getLongKey() +
                                        "' access error, use multiple getter");
    }
}

const std::vector<std::string> &ParamString::getMultipleValue() const {
    if (isMultipleValue()) {
        if (isValueSet()) {
            return m_values;
        } else {
            return m_defaultValues;
        }
    } else {
        throw InvalidParameterException("Option '" + getLongKey() +
                                        "' access error, use regular getter");
    }
}

void ParamString::setValue(const std::string &value) {
    if (isMultipleValue()) {
        m_values = parseValuesToVector(value);
    } else {
        m_values.clear();
        m_values.push_back(value);
    }

    validate();
    setValueSet();
}

uint32_t ParamString::getMaxLength() const {
    return m_maxLength;
}

void ParamString::setMaxLength(uint32_t maxLength) {
    m_maxLength = maxLength;
}

void ParamString::setDefault(std::string value) {
    if (isMultipleValue()) {
        m_defaultValues = parseValuesToVector(value);
    } else {
        m_defaultValues.clear();
        m_defaultValues.push_back(value);
    }

    validate();
    m_hasDefaultValue = true;
}

bool ParamString::hasDefaultValue() const {
    return m_hasDefaultValue;
}

void ParamString::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::FieldDescriptor *fieldDescriptor) {
    if (isMultipleValue()) {
        if (!fieldDescriptor->is_repeated()) {
            throw InvalidParameterException("Option '" + getLongKey() +
                                            "' cannot be parsed to protocol "
                                            "buffer, multiple value mismatch");
        }

        const auto &values = getMultipleValue();
        for (const auto &value : values) {
            message->GetReflection()->AddString(message, fieldDescriptor,
                                                value);
        }
    } else {
        message->GetReflection()->SetString(message, fieldDescriptor,
                                            getValue());
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
        m_multipleValueLimit = paramOps.cli_str().repeated_limit();
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

void ParamString::validate() const {
    if (isMultipleValue()) {
        const auto &values = getMultipleValue();

        if (m_multipleValueLimit && m_multipleValueLimit < values.size()) {
            throw InvalidParameterException(
                    "Limit of multiple values of option '" + getLongKey() +
                    "' exceed.");
        }

        for (const auto &value : values) {
            if (value.length() > m_maxLength) {
                throw InvalidParameterException(
                        "Multiple value's item length of option '" +
                        getLongKey() + "' is too long.");
            }
        }
    } else {
        const auto &value = getValue();
        if (value.length() > m_maxLength) {
            throw InvalidParameterException("Value length of option '" +
                                            getLongKey() + "' is too long.");
        }
    }
}

}  // namespace cli
}  // namespace octf
