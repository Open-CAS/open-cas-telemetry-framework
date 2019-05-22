/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_PARAM_PARAMSTRING_H
#define SOURCE_OCTF_CLI_PARAM_PARAMSTRING_H

#include <string>
#include <octf/cli/CLIElement.h>
#include <octf/cli/param/Parameter.h>

namespace octf {

constexpr int MAX_STRING_LENGTH = 4096;

/**
 * @brief String parameter class
 */
class ParamString : public Parameter {
public:
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

    /**
     * @brief Gets value(s) of this parameter
     * @return Parameter value
     */
    const std::vector<std::string> &getValue() const;

    /**
     * @brief Sets value of this parameter
     * @param value Value(s) to be set (can be delimited
     * multiple values)
     */
    void setValue(std::string value);

    /**
     * @brief Checks if this parameter value is valid
     * @param value Value to validate
     * @return If this parameter value is valid
     */
    static bool isValid(const std::string &value);

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

    virtual void setOptions(const proto::CliParameter &paramDef) override;

    virtual void setMultipleValue(bool multipleValue) override;

    virtual bool isMultipleValue() const override;

    void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) override;

protected:
    std::vector<std::string> parseValuesToVector(std::string values) const;

private:
    std::vector<std::string> m_values;
    std::vector<std::string> m_defaultValue;
    uint32_t m_maxLength;
    bool m_multipleValue;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_PARAM_PARAMSTRING_H
