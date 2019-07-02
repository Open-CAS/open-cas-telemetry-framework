/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMETER_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMETER_H

#include <octf/cli/internal/param/IParameter.h>
#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace cli {

class Parameter : public IParameter, public NonCopyable {
public:
    Parameter();

    virtual ~Parameter() = default;

    void getHelp(std::stringstream &ss) const override;

    bool isRequired() const override;

    /**
     * @note By default don't support multiple value
     */
    bool isMultipleValue() const override;

    bool isValueSet() const override;

    const std::string &getShortKey() const override;

    const std::string &getLongKey() const override;

    const std::string &getDesc() const override;

    const int32_t &getIndex() const override;

    const std::string &getWhat() const override;

    bool hasDefaultValue() const override;

    bool isHidden() const override;

    bool hasValue() const override;

    void setRequired(bool required) override;

    void setMultipleValue(bool repeated) override;

    void setDesc(const std::string &desc) override;

    void setIndex(const int32_t index) override;

    void setShortKey(const std::string &key) override;

    void setLongKey(const std::string &key) override;

    void setWhat(const std::string &what) override;

    void setHidden(bool hidden) override;

    virtual void setValueSet();

    virtual void setOptions(
            const google::protobuf::FieldDescriptor *fieldDesc) override;

    virtual void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) {
        // void cast in order to omit compilation warning
        (void) message;
        (void) fieldDescriptor;

        throw Exception(
                "Parameter doesn't support communication "
                "with plugin/service.");
    }

private:
    std::string m_shortKey;
    std::string m_longKey;
    std::string m_desc;
    std::string m_what;
    int32_t m_index;
    bool m_set;
    bool m_required;
    bool m_hidden;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMETER_H
