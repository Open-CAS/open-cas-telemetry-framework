/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/OptionsValidation.h>

namespace octf {
namespace cli {

const std::string &CLIElement::getValue() const {
    return m_value;
}

bool CLIElement::hasPrefix(const std::string &text, const std::string &prefix) {
    if ((text.empty() && !prefix.empty()) ||
        (prefix.length() > text.length())) {
        // Input text is empty or shorter than specified prefix.
        return false;
    }
    // Compare the beginning of string with whole prefix.
    bool result = text.compare(0, prefix.length(), prefix);
    // String compare returns 0 for equal strings, thus return negation.
    return !result;
}

std::string CLIElement::parseKey(const std::string &key,
                                 const std::string &prefix) {
    std::string validKey = "";
    if (hasPrefix(key, prefix)) {
        validKey = key.substr(prefix.size(), key.size());
    }
    return validKey;
}

std::string CLIElement::getKeyName() {
    std::string key = parseKey(m_value, LONG_KEY_PREFIX);
    if (key.empty()) {
        // Element is not a valid long key.
        // Try to interpret it as a short key.
        key = parseKey(m_value, SHORT_KEY_PREFIX);
        if (key.empty()) {
            m_type = CliType::Other;
        } else {
            // Valid short key
            m_type = CliType::ShortKey;
        }
    } else {
        // Valid long key
        m_type = CliType::LongKey;
    }

    return key;
}

CliType CLIElement::getType() {
    return m_type;
}

bool CLIElement::isValidKey(std::string key, CliType keyType) {
    bool result = false;

    if (keyType == CliType::ShortKey) {
        result = utils::isShortKeyValid(key);
    } else if (keyType == CliType::LongKey) {
        result = utils::isLongKeyValid(key);
    }

    return result;
}

std::string CLIElement::getValidKeyName() {
    std::string key = getKeyName();
    if (key.empty() || !isValidKey(key, getType())) {
        // Return empty object
        return {};
    }
    return key;
}

void CLIElement::setValue(const std::string &value) {
    m_value = value;
}

bool CLIElement::operator==(const std::string &value) const {
    if (m_value == value) {
        return true;
    } else {
        return false;
    }
}

}  // namespace cli
}  // namespace octf
