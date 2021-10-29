/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMFLAG_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMFLAG_H

#include <octf/cli/internal/param/Parameter.h>

namespace octf {
namespace cli {

class CLIElement;

/**
 * @brief This parameter represents a flag which takes no value
 */
class ParamFlag : public Parameter {
public:
    ParamFlag()
            : Parameter() {
        setWhat("");
    }
    virtual ~ParamFlag() = default;

    bool hasValue() const override {
        return false;
    }

    void setValue(CLIElement element) override {
        // Element argument not used (parameter without value)
        // Omit compilation warning
        (void) element;

        setValueSet();
    };

    /**
     * @brief Get value of this flag parameter
     * @return If this flag has been set
     */
    bool getValue() {
        return isValueSet();
    }

    void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) override {
        message->GetReflection()->SetBool(message, fieldDescriptor, getValue());
    }
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMFLAG_H
