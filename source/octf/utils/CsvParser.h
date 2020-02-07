/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
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

/**
 * @brief Parser for CSV content
 *
 * @note Commas and newlines escaping is not handled
 * @note Requires strict CSV format:
 *  - number of record elements must be <= number of header elements,
 *  - comma (',')delimited,
 *  - no empty header elements allowed,
 *  - no duplicate header elements
 *
 * @note Throws exception upon parsing errors
 */
class CsvParser {
public:
    CsvParser();
    virtual ~CsvParser() = default;

    void parseCsv(std::istream &content,
                  const std::list<std::string> &requiredHeaderElements);

    void parseCsv(const std::string &content,
                  const std::list<std::string> &requiredHeaderElements);

    octf::table::Table &getParsedContent() {
        return m_data;
    }

    static constexpr long long getMaxContentSize() {
        return m_maxContentSize;
    }

private:
    constexpr static long long m_maxContentSize = 10 * 1024 * 1024;  // 10MiB
    octf::table::Table m_data;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CSVPARSER_H
