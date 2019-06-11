/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_PARAM_PARAMYESNO_H
#define SOURCE_OCTF_CLI_PARAM_PARAMYESNO_H

#include <octf/cli/internal/param/ParamEnum.h>

namespace octf {

/**
 * @brief Yes/No parameter
 */
class ParamYesNo : public ParamEnum {
public:
    /**
     * @brief Enum for yes and no options
     */
    enum { Yes = 1, No = 0 };

    /**
     * @brief Parameter constructor
     */
    ParamYesNo();

    virtual ~ParamYesNo() = default;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_PARAM_PARAMYESNO_H
