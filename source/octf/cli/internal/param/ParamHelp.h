/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMHELP_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMHELP_H

#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/param/Parameter.h>

namespace octf {
namespace cli {

/**
 * @brief Help parameter class
 */
class ParamHelp : public Parameter {
public:
    /**
     * @brief Help parameter constructor
     */
    ParamHelp();

    virtual ~ParamHelp() = default;

    /**
     * @brief Parses input for help parameter
     */
    void setValue(CLIElement value) override;

    bool hasValue() const override;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMHELP_H
