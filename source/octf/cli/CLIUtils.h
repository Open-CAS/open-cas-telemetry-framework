/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CLIUTILS_H
#define SOURCE_OCTF_CLI_CLIUTILS_H

#include <google/protobuf/descriptor.h>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/CommandSet.h>
#include <octf/cli/Module.h>

constexpr int64_t kB = (1000ULL);
constexpr int64_t MB = (1000ULL * kB);
constexpr int64_t GB = (1000ULL * MB);
constexpr int64_t TB = (1000ULL * GB);

constexpr int64_t KiB = (1024ULL);
constexpr int64_t MiB = (1024ULL * KiB);
constexpr int64_t GiB = (1024ULL * MiB);
constexpr int64_t TiB = (1024ULL * GiB);

#define STR_VALUE(val) #val
#define TOSTR(name) STR_VALUE(name)

namespace octf {

typedef std::shared_ptr<google::protobuf::Message> MessageShRef;

/**
 * @brief Class sharing CLI utilities.
 *
 * Functionalities of this class are mostly helper methods,
 * useful for string conversion, file operations, and CLI printing
 */
namespace cliUtils {

/**
 * @brief Converts a given unsigned integer to string representing bytes.
 * @param bytes Number of bytes
 * @param base [optional] Unit of bytes to be converted to.
 *  If none supplied, the most appropriate one is returned
 * @return String containing bytes in given unit
 */
std::string bytes2string(uint64_t bytes, uint64_t base = 0);

/**
 * @brief Converts a double to string with given precision
 * @param f Floating point value
 * @param precision Precision of value string representation
 * @return String with floating point value of given precision
 */
std::string double2string(double f, uint32_t precision = 2);

/**
 * @brief Prints progress bar
 * @param prog Progress bar progress as a fraction
 * @param out Output stream for printing
 */
void printProgressBar(double progress, std::ostream &out);

/**
 * @brief Prints progress bar
 * @param prog Progress bar progress as a fraction
 * @return String containing progress bar
 */
std::string printProgressBar(double progress);

/**
 * @brief Prints protobuf message
 *
 * @param message Message to be printed
 */
void printOutputMessage(MessageShRef message);

/**
 * @brief Prints specified keys and description to the stringstream
 *
 * @param ss Stringstream to print to
 * @param shortKey Short Key (can be empty)
 * @param longKey Long Key
 * @param desc Description
 * @param isList Format specifier. If true, format is adjusted to the list view,
 * especially there is a new line at the end.
 */
void printKeys(std::stringstream &ss,
               std::string shortKey,
               std::string longKey,
               std::string desc = "",
               bool isList = true);

/**
 * @brief Prints usage hint for the module (plugin) to the stringstream
 *
 * @param ss Stringstream to print to
 * @param module Module (plugin) to get usage hint for (if nullptr is passed,
 * usage hint has generic form)
 * @param isList Format specifier. If true, format is adjusted to the list view,
 * especially there is a new line at the end.
 * @param hasPlugins Indicates that CLI can execute command on plugins
 * @param cliProperties CLI Properties
 */
void printUsage(std::stringstream &ss,
                Module *module,
                const CLIProperties &cliProperties,
                bool isList = false,
                bool hasPlugins = false);

/**
 * @brief Prints short help for Module (plugin) to the stringstream
 * @param ss Stringstream to print to
 * @param module Module (plugin) to get help for
 * @param isList Format specifier. If true, format is adjusted to the list view,
 * especially there is a new line at the end.
 *
 * This method doesn't print complete help for commands supported by the module,
 * just information about the module itself (short, long key, etc.)
 */
void printModuleHelp(std::stringstream &ss, Module *module, bool isList);

/**
 * @brief Prints help for Command Set to the stringstream
 * @param ss Stringstream to print to
 * @param cmdSet CommandSet to print help for
 */
void printCmdSetHelp(std::stringstream &ss, const CommandSet &cmdSet);

/**
 * @brief Prints help for Command to the stringstream
 * @param ss Stringstream to print to
 * @param cmd Command to print help for
 * @param cliProperties CLI Properties
 */
void printCmdHelp(std::stringstream &ss,
                  std::shared_ptr<ICommand> cmd,
                  const CLIProperties &cliProperties);

}  // namespace cliUtils
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_CLIUTILS_H
