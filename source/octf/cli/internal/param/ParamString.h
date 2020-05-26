/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMSTRING_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMSTRING_H

#include <string>
#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/param/Parameter.h>

namespace octf {
namespace cli {

constexpr int MAX_STRING_LENGTH = 4096;

/**
 * This value specifies delimiter for multiple value parameters
 */
constexpr char PARAMETER_VALUE_DELIMITER[] = ",";

/**
 * @brief String parameter class
 */
class ParamString : public Parameter {
public:
    // TODO(trybicki): Implement printing default value in help
    /**
     * @brief Parameter constructor
     */
    ParamString();

    virtual ~ParamString() = default;

    /**
     * @brief Parses input for parameter
     */
    virtual void setValue(CLIElement element) override;

    /**
     * @brief Set default value of this parameter
     * @param value Default value of this parameter
     */
    virtual void setDefault(std::string value);

    bool hasDefaultValue() const override;

    /**
     * @brief Gets value(s) of this parameter
     * @return Parameter value
     */
    const std::string &getValue() const;

    /**
     * @brief Gets value(s) of this parameter
     * @return Parameter value
     */
    const std::vector<std::string> &getMultipleValue() const;

    /**
     * @brief Sets value of this parameter
     * @param value Value(s) to be set (can be delimited
     * multiple values)
     */
    void setValue(const std::string &value);

    /**
     * @brief Gets maximal length of parameter value
     * @return Maximal length of parameter value
     */
    uint32_t getMaxLength() const;

    /**
     * @brief Sets maximal length of parameter value
     * @param maxLength maximal length of parameter value
     */
    void setMaxLength(uint32_t maxLength);

    virtual void setOptions(
            const google::protobuf::FieldDescriptor *fieldDesc) override;

    virtual void setMultipleValue(bool multipleValue) override;

    virtual bool isMultipleValue() const override;

    void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) override;

private:
    std::vector<std::string> parseValuesToVector(std::string values) const;

    /**
     * @brief Validates if value owned by string parameter is valid
     *
     * @throw InvalidParameterException in case of invalidation
     */
    void validate() const;

private:
    std::vector<std::string> m_values;
    std::vector<std::string> m_defaultValues;
    bool m_hasDefaultValue;
    uint32_t m_maxLength;
    bool m_multipleValue;
    uint32_t m_multipleValueLimit;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMSTRING_H
