/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMDOUBLE_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMDOUBLE_H

#include <cstdint>
#include <string>
#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/param/Parameter.h>

namespace octf {
namespace cli {

/**
 * @brief Number parameter class
 */
class ParamDouble : public Parameter {
public:
    /**
     * @brief Parameter constructor
     */
    ParamDouble();

    virtual ~ParamDouble() = default;

    virtual void setValue(CLIElement cliElement) override;

    /**
     * @brief Gets maximal value of this parameter
     * @return Maximal value of this parameter
     */
    double getMax() const;

    /**
     * @brief Sets maximal value of this parameter
     * @param Maximal value of this parameter
     */
    void setMax(double max);

    /**
     * @brief Gets minimal value of this parameter
     * @return Minimal value of this parameter
     */
    double getMin() const;

    /**
     * @brief Sets minimal value of this parameter
     * @param Minimal value of this parameter
     */
    void setMin(double min);

    /**
     * @brief Gets unit of this parameter
     * @return Unit of parameter
     */
    const std::string &getUnit() const;

    /**
     * @brief Sets unit of parameter
     * @param Unit of parameter
     */
    void setUnit(const std::string &unit);

    /**
     * @brief Gets double value of this parameter
     * @return Number value
     */
    double getDouble() const;

    /**
     * @brief Sets double value of this parameter
     * @param number Double value of this parameter
     */
    void setDouble(double number);

    /**
     * @brief Gets acceptable range of this parameter
     * @return Range of parameter values
     */
    std::string getRange() const;

    /**
     * @brief Gets default parameter value
     * @return Default parameter value
     */
    double getDefault() const;

    /**
     * @brief Sets default parameter value
     * @param _default Default value of parameter
     */
    void setDefault(double _default);

    /**
     * @brief Checks if this parameter has default value
     * @return If this parameter has default value
     */
    bool hasDefault() const;

    /**
     * @brief Gets description of this parameter
     * @return Parameter description
     */
    const std::string &getDesc() const override;

    /**
     * @brief Sets description of this parameter
     * @param desc Description of this parameter
     */
    void setDesc(const std::string &desc) override;

    /**
     * @brief Gets value of this parameter as a string
     * @return value of this parameter as a string
     */
    std::string toString() const;

    virtual void setOptions(
            const google::protobuf::FieldDescriptor *fieldDesc) override;

    void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) override;

private:
    void details();

private:
    double m_double;
    double m_min;
    double m_max;
    std::string m_unit;
    double m_default;
    bool m_hasDefault;
    std::string m_details;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMDOUBLE_H
