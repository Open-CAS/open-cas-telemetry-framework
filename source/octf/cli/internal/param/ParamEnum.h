/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMENUM_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMENUM_H

#include <map>
#include <string>
#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/param/Parameter.h>

extern "C" {
#include <errno.h>
}

namespace octf {
namespace cli {

/**
 * @brief Enum type parameter
 *
 * This class allows using enumerator type parameters.
 */
class ParamEnum : public Parameter {
public:
    /**
     * @brief Parameter constructor
     */
    ParamEnum();

    virtual ~ParamEnum() = default;

    /**
     * @brief Sets value of this parameter
     */
    void setValue(CLIElement element) override;

    /**
     * @brief Checks if given name and value pair exists in enum list
     * @param value Value of enum element
     * @param name Name of enum element
     * @return If value and name are in enum list
     */
    bool hasEnum(int64_t value, const std::string &name) const;

    /**
     * @brief Gets value of enum element name
     * @param name Name of enum element
     * @return Value of enum element
     */
    int64_t stringToValue(const std::string &name) const;

    /**
     * @brief  Gets name of enum element from enum element value
     * @param val value of enum element
     * @return name of enum element
     */
    std::string valueToString(int64_t val) const;

    /**
     * @brief Gets short enum list in curly brackets form
     * @return String with enum list
     */
    std::string getDescShort() const;

    /**
     * @brief Gets long enum list description
     * @return Long enum list description
     */
    std::string getDescLong() const;

    /**
     * @brief Gets value of this parameter
     * @return Enum element value
     */
    int64_t getValue() const;

    /**
     * @brief Gets default enum list element
     * @return Value of default enum list element
     */
    int64_t getDefault() const;

    /**
     * @brief Get string value of enum
     * @param value Value of enum
     * @return String value
     */
    std::string getStringValue(int64_t value) const;

    /**
     * @brief Set default enum list element value
     * @param _default
     */
    void setDefault(int64_t _default);

    /**
     * @brief Check if this enum list has a default element
     * @return If this enum list has a default element
     */
    bool hasDefault() const;

    /**
     * @return Description of enum parameter
     */
    const std::string &getDesc() const override;

    /**
     * @brief Get description of this enum
     * @param desc
     */
    void setDesc(const std::string &desc);

    //    virtual void setOptions(google::protobuf::FieldDescriptor* fieldDesc)
    //    override;

    void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) override;

private:
    void details();

    class Item {
    public:
        Item(int64_t value, const std::string &name, const std::string &desc)
                : m_value(value)
                , m_name(name)
                , m_desc(desc) {}

        Item(const Item &item)
                : m_value(item.m_value)
                , m_name(item.m_name)
                , m_desc(item.m_desc) {}

        Item &operator=(const Item &item) {
            if (this == &item) {
                return *this;
            }

            this->m_value = item.m_value;
            this->m_name = item.m_name;
            this->m_desc = item.m_desc;

            return *this;
        }

        Item()
                : m_value(0)
                , m_name("")
                , m_desc("") {}

        const std::string &getDesc() const {
            return m_desc;
        }

        const std::string &getName() const {
            return m_name;
        }

        int64_t getValue() const {
            return m_value;
        }

    private:
        int64_t m_value;
        std::string m_name;
        std::string m_desc;
    };

public:
    void addEnumVal(int64_t value,
                    const std::string &name,
                    const std::string &desc);

private:
    std::map<int64_t, Item> m_enums;
    int64_t m_value;
    int64_t m_default;
    bool m_hasDefault;
    std::string m_details;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_PARAMENUM_H
