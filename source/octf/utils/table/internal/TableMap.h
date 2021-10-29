/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_INTERNAL_TABLEMAP_H
#define SOURCE_OCTF_UTILS_TABLE_INTERNAL_TABLEMAP_H

#include <stddef.h>
#include <map>
#include <string>
#include <octf/utils/NonCopyable.h>
#include <octf/utils/table/Addr.h>
#include <octf/utils/table/Iterators.h>
#include <octf/utils/table/Table.h>

namespace octf {
namespace table {

/**
 * @brief Cells' Map for the magic table (Table)
 *
 * This class is the map of cells keyed by addresses [row, column]. In addition
 * it keeps an association of column/row names and their indexes, and returns
 * Table iterators.
 */
class TableMap {
public:
    TableMap();
    virtual ~TableMap();
    TableMap(TableMap const &other);
    TableMap &operator=(TableMap const &other);

    index_t getRowCount() const;

    index_t getColumnCount() const;

    index_t getRowAssociation(const std::string &name);

    index_t getColumnAssociation(const std::string &name);

    Cell &operator[](const Addr &addr);

    const Cell &operator[](const Addr &addr) const;

    Row &getRow(index_t index);

    const Row &getRow(index_t index) const;

    Column &getColumn(index_t index);

    RowIterator beginRow();
    RowIterator endRow();

    RowIteratorConst beginRow() const;
    RowIteratorConst endRow() const;

    CellIterator beginCell(index_t id);
    CellIterator endCell(index_t id);
    CellIterator eraseCell(CellIterator &iter);

    CellIteratorConst beginCell(index_t id) const;
    CellIteratorConst endCell(index_t id) const;

    void clear();
    index_t size() const;
    bool empty() const;

    const std::map<std::string, index_t> &getColumnsAssociation() const {
        return m_columnsAssociation;
    }

    const std::map<std::string, index_t> &getRowsAssociation() const {
        return m_rowsAssociation;
    }

private:
    std::map<index_t, Row> m_rows;
    std::map<std::string, index_t> m_rowsAssociation;
    std::map<index_t, Column> m_columns;
    std::map<std::string, index_t> m_columnsAssociation;
    std::map<Addr, Cell> m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_INTERNAL_TABLEMAP_H
