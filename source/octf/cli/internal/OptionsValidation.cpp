/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/OptionsValidation.h>

#include <ctype.h>

namespace octf {
namespace cli {
namespace utils {

bool isShortKeyValid(const std::string &key) {
    // Valid short key is a single alphabetic char: [a-zA-Z] )

    if (key.empty()) {
        // Short key not specified
        return true;
    }

    if (key.length() != 1) {
        // Key too short or too long
        return false;
    }
    if (!isalpha(key[0])) {
        // Not an alpha char
        return false;
    }
    // All checks passed successfully, short key is valid.
    return true;
}
bool isLongKeyValid(const std::string &key) {
    // Valid long key can contain chars [a-zA-Z] and "-"
    // The first char has to be a letter

    if (key.length() < 1) {
        // Key too short (empty)
        return false;
    }

    for (size_t i = 0; i < key.length(); i++) {
        if (!((isalnum(key[i])) || ((key[i] == '-') && (i > 0)))) {
            // Disallowed character
            return false;
        }
    }
    // All checks passed successfully, long key is valid.
    return true;
}
bool isDescValid(const std::string &desc) {
    // Description can contain any number of printable characters

    if (desc.length() < 1) {
        // Description too short (empty)
        return false;
    }
    for (size_t i = 0; i < desc.length(); i++) {
        if (!isprint(desc[i])) {
            // Disallowed (not printable) character
            return false;
        }
    }
    // All checks passed successfully, description is valid.
    return true;
}

bool isCommandSetValid(const proto::CliCommandSet &commandSet,
                       bool validCommandsAssured) {
    // Local containers for keeping already used keys of methods;
    // used to check uniqueness in whole CommandSet
    std::set<std::string> longKeys;
    std::set<std::string> shortKeys;
    for (int i = 0; i < commandSet.command().size(); i++) {
        const proto::CliCommand &cmd = commandSet.command(i);
        auto const &longKey = cmd.cmdops().cli_long_key();
        auto const &shortKey = cmd.cmdops().cli_short_key();

        if (!validCommandsAssured && !isCommandValid(cmd)) {
            return false;
        }

        // Check if long key is unique
        if (longKeys.find(longKey) != longKeys.end()) {
            // Long key already used, thus whole description
            // becomes ambiguous
            return false;
        }
        // Key is unique as for now, add it to container for further comparisons
        longKeys.emplace(longKey);

        if (shortKey.empty()) {
            // If no short key specified just continue
            continue;
        }

        // Check if short key is unique
        if (shortKeys.find(shortKey) != shortKeys.end()) {
            // Short key already used, thus whole description
            // become ambiguous
            return false;
        }
        // Key is unique as for now, keep it for further comparisons
        shortKeys.emplace(shortKey);
    }
    return true;
}

bool isCommandValid(const proto::CliCommand &cmd) {
    auto const &longKey = cmd.cmdops().cli_long_key();
    auto const &shortKey = cmd.cmdops().cli_short_key();
    auto const &desc = cmd.cmdops().cli_desc();

    bool result = isLongKeyValid(longKey) &&
                  (isShortKeyValid(shortKey) || shortKey.empty()) &&
                  isDescValid(desc);

    return result;
}

}  // namespace utils
}  // namespace cli
}  // namespace octf
