/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <climits>
#include <cstdint>
#include <exception>
#include <sstream>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/param/ParamNumber.h>
#include <octf/cli/internal/param/Parameter.h>
#include <octf/utils/Exception.h>

extern "C" {
#include <errno.h>
}
using namespace std;

namespace octf {
namespace cli {

ParamNumber::ParamNumber()
        : Parameter()
        , m_number(0)
        , m_min(INT64_MIN)
        , m_max(INT64_MAX)
        , m_unit("")
        , m_default(0)
        , m_hasDefault(false)
        , m_details("") {}

void ParamNumber::setValue(CLIElement cliElement) {
    if (isValueSet()) {
        throw InvalidParameterException("Option '" + getLongKey() +
                                        "' double set.");
    }

    if (cliElement == "") {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is missing.");
    }

    istringstream ss(cliElement.getValue());
    ss.imbue(std::locale::classic());
    ss >> m_number;

    if (ss.fail() || (false == ss.eof())) {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is invalid.");
    }

    if (m_number < m_min || m_number > m_max) {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is out of range.");
    }

    setValueSet();
}

string ParamNumber::getRange() const {
    stringstream range;
    range << "<" << m_min << "-" << m_max << ">";
    return range.str();
}

string ParamNumber::toString() const {
    stringstream ss;
    ss << m_number;
    return ss.str();
}

void ParamNumber::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::FieldDescriptor *fieldDescriptor) {
    switch (fieldDescriptor->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32: {
        // This is generic number type based on INT64. We have to check
        // if number value is in range of INT32
        std::numeric_limits<int32_t> limit;

        if (m_number >= limit.min() && m_number <= limit.max()) {
            message->GetReflection()->SetInt32(message, fieldDescriptor,
                                               m_number);
        } else {
            throw InvalidParameterException("Number value out of range.");
        }
        break;
    }

    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        message->GetReflection()->SetInt64(message, fieldDescriptor, m_number);
        break;

    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32: {
        // This is generic number type which based on INT64. We have to check
        // if number value is in range of UINT32
        std::numeric_limits<uint32_t> limit;

        if (m_number >= limit.min() && m_number <= limit.max()) {
            message->GetReflection()->SetUInt32(message, fieldDescriptor,
                                                m_number);
        } else {
            throw InvalidParameterException("Number value out of range.");
        }
        break;
    }

    default:
        throw InvalidParameterException("Number value out of range.");
        break;
    }
}

void ParamNumber::details() {
    stringstream ss;

    ss << Parameter::getDesc();

    if (false == hasDefault() && "" == getUnit()) {
        m_details = ss.str();
        return;
    }

    ss << " (";

    if (hasDefault()) {
        ss << "default: " << m_default;

        if ("" != getUnit()) {
            ss << ", ";
        }
    }

    if ("" != getUnit()) {
        ss << "unit: " << m_unit;
    }

    ss << ")";

    m_details = ss.str();
}

int64_t ParamNumber::getMax() const {
    return m_max;
}

void ParamNumber::setMax(int64_t max) {
    m_max = max;
    setWhat(getRange());
}

int64_t ParamNumber::getMin() const {
    return m_min;
}

void ParamNumber::setMin(int64_t min) {
    m_min = min;
    setWhat(getRange());
}

const std::string &ParamNumber::getUnit() const {
    return m_unit;
}

void ParamNumber::setUnit(const std::string &unit) {
    m_unit = unit;
    details();
}

int64_t ParamNumber::getNumber() const {
    return m_number;
}

void ParamNumber::setNumber(int64_t number) {
    m_number = number;
}

int64_t ParamNumber::getDefault() const {
    return m_default;
}

void ParamNumber::setDefault(int64_t _default) {
    m_default = _default;
    m_hasDefault = true;
    m_number = _default;
    details();
}

bool ParamNumber::hasDefault() const {
    return m_hasDefault;
}

const std::string &ParamNumber::getDesc() const {
    return m_details;
}

void ParamNumber::setDesc(const std::string &desc) {
    Parameter::setDesc(desc);
    details();
}

void ParamNumber::setOptions(const proto::CliParameter &paramDef) {
    using namespace google::protobuf;
    const proto::OptsParam &paramOps = paramDef.paramops();

    // Set options independent of type
    Parameter::setOptions(paramDef);

    if (paramOps.has_cli_num()) {
        // If number-specific options are defined, set them
        setDefault(paramOps.cli_num().default_value());
        setMin(paramOps.cli_num().min());
        setMax(paramOps.cli_num().max());

    } else {
        // No options defined, set default values
        switch (paramDef.type()) {
        case proto::CliParameter::Type::CliParameter_Type_INT32: {
            std::numeric_limits<int32_t> limit;
            setMin(limit.min());
            setMax(limit.max());
            setDefault(0);
            break;
        }

        case proto::CliParameter::Type::CliParameter_Type_INT64: {
            std::numeric_limits<int64_t> limit;
            setMin(limit.min());
            setMax(limit.max());
            setDefault(0);
            break;
        }

        case proto::CliParameter::Type::CliParameter_Type_UINT32: {
            std::numeric_limits<uint32_t> limit;
            setMin(limit.min());
            setMax(limit.max());
            setDefault(0);
            break;
        }

        default:
            throw InvalidParameterException("Unsupported numeric type.");
            break;
        }
    }
}

}  // namespace cli
}  // namespace octf
