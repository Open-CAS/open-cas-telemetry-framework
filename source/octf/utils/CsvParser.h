/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_CSVPARSER_H
#define SOURCE_OCTF_UTILS_CSVPARSER_H

#include <algorithm>
#include <istream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <octf/utils/table/Table.h>

namespace octf {
namespace csvparser {

/**
 * @file Parser utils for parsing CSV content
 *
 * @note Commas and newlines escaping is not handled
 *
 * @note Requires strict CSV format:
 *  - number of record elements must be <= number of header elements,
 *  - comma (',')delimited,
 *  - no empty header elements allowed,
 *  - no duplicate header elements
 *
 * @note Throws exception upon parsing errors
 */

/**
 * @brief Parse csv
 *
 * @param content Stream with csv content, read from set position
 * @param requiredHeaderElements Header elements which must be present
 *  otherwise Exception is thrown
 * @param[out] parsed Table which will be set with parsed CSV content.
 *
 * @note parsed[0] shall contain parsed header row, and consequtive table rows
 *  shall contain parsed records.
 */
void parseCsv(std::istream &content,
              const std::list<std::string> &requiredHeaderElements,
              octf::table::Table &parsed);

/**
 * @brief Parse csv
 *
 * @param content String with csv content
 * @param requiredHeaderElements Header elements which must be present
 *  otherwise Exception is thrown
 * @param[out] parsed Table which will be set with parsed CSV content.
 *
 * @note parsed[0] shall contain parsed header row, and consequtive table rows
 *  shall contain parsed records.
 */
void parseCsv(const std::string &content,
              const std::list<std::string> &requiredHeaderElements,
              octf::table::Table &parsed);

constexpr static long long MAX_CSV_CONTENT_SIZE = 10 * 1024 * 1024;  // 10MiB

};  // namespace csvparser
};  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CSVPARSER_H
