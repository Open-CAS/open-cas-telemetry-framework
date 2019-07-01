/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
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

class TableMap : public NonCopyable {
public:
    TableMap();
    virtual ~TableMap();

    size_t getRowCount() const;

    size_t getColumnCount() const;

    size_t getRowAssociation(const std::string &name);

    size_t getColumnAssociation(const std::string &name);

    Cell &operator[](const Addr &addr);

    const Cell &operator[](const Addr &addr) const;

    Row &getRow(size_t index);

    const Row &getRow(size_t index) const;

    Column &getColumn(size_t index);

    RowIterator beginRow();
    RowIterator endRow();

    RowIteratorConst beginRow() const;
    RowIteratorConst endRow() const;

    CellIterator beginCell(size_t id);
    CellIterator endCell(size_t id);
    CellIterator eraseCell(CellIterator iter);

    CellIteratorConst beginCell(size_t id) const;
    CellIteratorConst endCell(size_t id) const;

    void clear();
    size_t size() const;
    bool empty() const;

private:
    std::map<size_t, Row> m_rows;
    std::map<std::string, size_t> m_rowsAssociation;
    std::map<size_t, Column> m_columns;
    std::map<std::string, size_t> m_columnsAssociation;
    std::map<Addr, Cell> m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_INTERNAL_TABLEMAP_H
