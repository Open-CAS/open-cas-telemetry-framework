/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CLIELEMENT_H
#define SOURCE_OCTF_CLI_INTERNAL_CLIELEMENT_H

#include <string>

namespace octf {
namespace cli {

/**
 * @type Possible types of single CLI input element
 */
enum class CliType { Raw, ShortKey, LongKey, Other };
/**
 * @brief Class holding Command Line Interface element (argument) value
 *
 * Generic class for any CLI element. CLI element is
 * any space delimited element of program's command line input
 *
 * In example:
 * "./applicationName --command --paramName paramValue1 -s paramValue2"
 *
 * the "--command", "--paramName", "paramValue1",
 * "-s" and "paramValue2" are all CLI elements
 */
class CLIElement {
public:
    /**
     * @brief Creates empty CLIElement
     */
    CLIElement()
            : m_value("")
            , m_type(CliType::Raw) {}

    /**
     * @brief Creates CLIElement with given value
     * @param value Value of command line element
     */
    CLIElement(std::string value)
            : m_value(value)
            , m_type(CliType::Raw) {}

    virtual ~CLIElement() = default;

    /**
     * @return Value of CLIElement
     */
    const std::string &getValue() const;

    CliType getType();

    std::string getValidKeyName();

    /**
     * @brief Sets value of CLI element
     * @param value Value of CLI element
     */
    void setValue(const std::string &value);

    bool operator==(const std::string &value) const;

private:
    bool hasPrefix(const std::string &text, const std::string &prefix);

    std::string parseKey(const std::string &key, const std::string &prefix);
    /**
     * @brief Gets (creates) the key name, removing appropriate prefix
     *
     * @return Name of the key with prefix removed
     * Empty string if no key prefix was found
     */
    std::string getKeyName();

    bool isValidKey(std::string key, CliType keyType);

private:
    std::string m_value;
    CliType m_type;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CLIELEMENT_H
