/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <climits>
#include <cstdint>
#include <exception>
#include <sstream>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/param/ParamDouble.h>
#include <octf/cli/internal/param/Parameter.h>
#include <octf/utils/Exception.h>

extern "C" {
#include <errno.h>
}
using namespace std;

namespace octf {
namespace cli {

ParamDouble::ParamDouble()
        : Parameter()
        , m_double(0)
        , m_min(-numeric_limits<double>::max())
        , m_max(numeric_limits<double>::max())
        , m_unit("")
        , m_default(0)
        , m_hasDefault(false)
        , m_details("") {}

void ParamDouble::setValue(CLIElement cliElement) {
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
    ss >> m_double;

    if (ss.fail() || (false == ss.eof())) {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is invalid.");
    }

    if (m_double < m_min || m_double > m_max) {
        throw InvalidParameterException("Value of option '" + getLongKey() +
                                        "' is out of range.");
    }

    setValueSet();
}

string ParamDouble::getRange() const {
    stringstream range;
    range << "<" << m_min << "-" << m_max << ">";
    return range.str();
}

string ParamDouble::toString() const {
    stringstream ss;
    ss << m_double;
    return ss.str();
}

void ParamDouble::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::FieldDescriptor *fieldDescriptor) {
    switch (fieldDescriptor->cpp_type()) {
    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE: {
        // This is generic number type based on INT64. We have to check
        // if number value is in range of INT32
        std::numeric_limits<double> limit;

        if (m_double >= -limit.max() && m_double <= limit.max()) {
            message->GetReflection()->SetDouble(message, fieldDescriptor,
                                                m_double);
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

void ParamDouble::details() {
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

double ParamDouble::getMax() const {
    return m_max;
}

void ParamDouble::setMax(double max) {
    m_max = max;
    setWhat(getRange());
}

double ParamDouble::getMin() const {
    return m_min;
}

void ParamDouble::setMin(double min) {
    m_min = min;
    setWhat(getRange());
}

const std::string &ParamDouble::getUnit() const {
    return m_unit;
}

void ParamDouble::setUnit(const std::string &unit) {
    m_unit = unit;
    details();
}

double ParamDouble::getDouble() const {
    return m_double;
}

void ParamDouble::setDouble(double number) {
    m_double = number;
}

double ParamDouble::getDefault() const {
    return m_default;
}

void ParamDouble::setDefault(double _default) {
    m_default = _default;
    m_hasDefault = true;
    m_double = _default;
    details();
}

bool ParamDouble::hasDefault() const {
    return m_hasDefault;
}

const std::string &ParamDouble::getDesc() const {
    return m_details;
}

void ParamDouble::setDesc(const std::string &desc) {
    Parameter::setDesc(desc);
    details();
}

void ParamDouble::setOptions(
        const google::protobuf::FieldDescriptor *fieldDesc) {
    using namespace google::protobuf;

    const google::protobuf::FieldOptions &fieldOptions = fieldDesc->options();

    if (!fieldOptions.HasExtension(proto::opts_param)) {
        throw Exception("Input message's field: " + fieldDesc->name() +
                        " does not have parameter options");
    }
    const proto::OptsParam &paramOps =
            fieldOptions.GetExtension(proto::opts_param);

    // Set options independent of type
    Parameter::setOptions(fieldDesc);

    if (paramOps.has_cli_double()) {
        // If number-specific options are defined, set them
        setDefault(paramOps.cli_double().default_value());
        setMin(paramOps.cli_double().min());
        setMax(paramOps.cli_double().max());

    } else {
        // No options defined, set default values
        switch (fieldDesc->type()) {
        case FieldDescriptor::Type::TYPE_DOUBLE: {
            std::numeric_limits<double> limit;
            setMin(-limit.max());
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
