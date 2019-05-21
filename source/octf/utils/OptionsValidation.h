/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_OPTIONSVALIDATION_H
#define SOURCE_OCTF_UTILS_OPTIONSVALIDATION_H
#include <string>
#include <octf/proto/InterfaceCLI.pb.h>

namespace octf {

constexpr char SHORT_KEY_PREFIX[] = "-";
constexpr char LONG_KEY_PREFIX[] = "--";

/**
 * Common functions to validate CLI options.
 * Can be used on server side when creating description of available interfaces
 * and on client side when parsing input from user
 */
namespace cliUtils {

/**
 * @brief Checks if specified argument is a valid short key.
 *
 * @param key Text argument to be checked
 *
 * Short key is optional thus validation of empty string returns true.
 *
 * In case of not empty string, requirements for valid short key:
 * 1-char length
 * an alpha char: [a-zA-Z]
 */
bool isShortKeyValid(const std::string &key);

/**
 * @brief Checks if specified argument is a valid long key.
 *
 * @param key Text argument to be checked
 *
 * Requirements for valid long key:
 * Non-empty
 * Can contain only alphanumeric chars or '-'
 * The first has to be alphanumeric ('-' not allowed)
 */
bool isLongKeyValid(const std::string &key);

/**
 * @brief Checks if specified argument is a valid description.
 *
 * @param desc Text argument to be checked
 *
 * Requirements for valid long description:
 * Non-empty
 * Can contain only printable chars
 */
bool isDescValid(const std::string &desc);

/**
 * @brief Checks if command is valid for CLI usage.
 *
 * @param cmd Command to be checked
 *
 * Requirements for valid command:
 * Has valid long key
 * Has valid short key or no short key (as it is optional)
 * Has valid description
 */
bool isCommandValid(const proto::CliCommand &cmd);

/**
 * @brief Checks if command set is valid for CLI usage.
 *
 * @param cmd Command set to be checked
 *
 * Requirements for valid command set:
 * Contains only valid commands
 * Uniqueness of long keys is assured
 * Uniqueness of short keys is assured
 */
bool isCommandSetValid(const proto::CliCommandSet &commandSet,
                       bool validCommandsAssured = false);
}  // namespace cliUtils
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_OPTIONSVALIDATION_H
