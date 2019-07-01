/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_TABLE_H
#define SOURCE_OCTF_UTILS_TABLE_TABLE_H

#include <map>
#include <string>
#include <octf/utils/Log.h>
#include <octf/utils/NonCopyable.h>
#include <octf/utils/container/IContainer.h>
#include <octf/utils/table/Cell.h>
#include <octf/utils/table/Column.h>
#include <octf/utils/table/Iterators.h>
#include <octf/utils/table/Row.h>

namespace octf {
namespace table {

class TableMap;

/**
 * @defgroup Table Table
 * The very flexible table assuring dynamic number or rows and columns
 *
 * @ingroup Utilities
 */

/**
 * @ingroup Table
 * @brief The very flexible table
 *
 * The table is capable to keep dynamically rows and columns. It provides double
 * array operator [][] for accessing cells. Cells may stores any type of value.
 *
 * @code
 * // Define the table
 * octf::table::Table table;
 *
 * // Fill table
 * table[0][0] = "A string";
 * table[1][1] = -1;
 * table[2][2] = 3.14;
 * table[0][0] = table[2][2];
 *
 * // Print table
 * octf::log << table << std::endl;
 * @endcode
 */
class Table : public NonCopyable,
              public IContainer<Row, RowIterator, RowIteratorConst> {
public:
    Table();
    virtual ~Table();

    /**
     * @brief Gets row by row index
     * @param row Row index
     * @return Row
     */
    Row &operator[](size_t row);

    /**
     * @brief Gets row by row name
     * @param row Row name
     * @return Row
     */
    Row &operator[](const std::string &row);

    /**
     * @brief Gets row by row index
     * @param row Row index
     * @return Row
     */
    const Row &operator[](size_t row) const;

    /**
     * @brief Gets row by row name
     * @param row Row name
     * @return Row
     */
    const Row &operator[](const std::string &row) const;

    void clear() override;

    size_t size() const override;

    bool empty() const override;

    RowIterator begin() override;

    RowIterator end() override;

    RowIteratorConst begin() const override;

    RowIteratorConst end() const override;

private:
    std::unique_ptr<TableMap> m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_TABLE_H
