/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/internal/TableMap.h>

#include <iostream>
#include <octf/utils/Exception.h>

namespace octf {
namespace table {

TableMap::TableMap()
        : NonCopyable()
        , m_rows()
        , m_rowsAssociation()
        , m_columns()
        , m_columnsAssociation()
        , m_map() {}

TableMap::~TableMap() {}

index_t TableMap::getRowAssociation(const std::string &name) {
    index_t index = m_rowsAssociation.size();
    auto iter = m_rowsAssociation.find(name);

    if (iter != m_rowsAssociation.end()) {
        index = iter->second;
    } else {
        m_rowsAssociation[name] = index;
    }

    return index;
}

index_t TableMap::getColumnAssociation(const std::string &name) {
    index_t index = m_columnsAssociation.size();
    auto iter = m_columnsAssociation.find(name);

    if (iter != m_columnsAssociation.end()) {
        index = iter->second;
    } else {
        m_columnsAssociation[name] = index;
    }

    return index;
}

Cell &TableMap::operator[](const Addr &addr) {
    auto iter = m_map.find(addr);

    if (iter == m_map.end()) {
        auto pair = std::make_pair(addr, Cell());

        auto result = m_map.emplace(pair);
        if (!result.second || result.first == m_map.end()) {
            throw Exception("Cannot insert cell to the table");
        }

        // Allocate Column
        getColumn(addr.getColumnIndex());

        return result.first->second;
    } else {
        return iter->second;
    }
}

const Cell &TableMap::operator[](const Addr &addr) const {
    auto iter = m_map.find(addr);

    if (iter == m_map.end()) {
        // Cell have not being accessed till now, for read only access we can
        // return empty cell
        static const Cell readOnlyEmptyCell;
        return readOnlyEmptyCell;
    } else {
        return iter->second;
    }
}

Row &TableMap::getRow(index_t index) {
    auto iter = m_rows.find(index);

    if (iter == m_rows.end()) {
        Row row(index, *this);
        auto pair = std::make_pair(index, row);

        auto result = m_rows.emplace(pair);
        if (false == result.second || result.first == m_rows.end()) {
            throw Exception("Cannot insert row to the table");
        }

        return result.first->second;
    } else {
        return iter->second;
    }
}

const Row &TableMap::getRow(index_t index) const {
    auto iter = m_rows.find(index);

    if (iter != m_rows.end()) {
        return iter->second;
    } else {
        throw Exception("Accessing non-existing row in table");
    }
}

Column &TableMap::getColumn(index_t index) {
    auto iter = m_columns.find(index);

    if (iter == m_columns.end()) {
        Column column(index, *this);
        auto pair = std::make_pair(index, column);

        auto result = m_columns.emplace(pair);
        if (false == result.second || result.first == m_columns.end()) {
            throw Exception("Cannot insert row to the table");
        }

        return result.first->second;
    } else {
        return iter->second;
    }
}

RowIterator TableMap::beginRow() {
    return RowIterator(m_rows.begin());
}

RowIterator TableMap::endRow() {
    return RowIterator(m_rows.end());
}

RowIteratorConst TableMap::beginRow() const {
    return RowIteratorConst(m_rows.begin());
}

RowIteratorConst TableMap::endRow() const {
    return RowIteratorConst(m_rows.end());
}

CellIterator TableMap::beginCell(index_t id) {
    return CellIterator(*this, m_columns.begin(), id);
}

CellIterator TableMap::endCell(index_t id) {
    return CellIterator(*this, m_columns.end(), id);
}

CellIterator octf::table::TableMap::eraseCell(CellIterator &iter) {
    if (iter.m_iter != m_columns.end()) {
        Addr addr(iter.m_rowIndex, iter.m_iter->first);
        m_map.erase(addr);

        iter.m_iter++;
        if (iter.m_iter == m_columns.end()) {
            // Last cell removed, cleanup this row entirely

            m_rows.erase(iter.m_rowIndex);
        }
    }

    return iter;
}

CellIteratorConst TableMap::beginCell(index_t id) const {
    return CellIteratorConst(*this, m_columns.begin(), id);
}

CellIteratorConst TableMap::endCell(index_t id) const {
    return CellIteratorConst(*this, m_columns.end(), id);
}

index_t TableMap::getRowCount() const {
    return m_rows.size();
}

index_t TableMap::getColumnCount() const {
    return m_columns.size();
}

void octf::table::TableMap::clear() {
    m_rows.clear();
    m_rowsAssociation.clear();
    m_columns.clear();
    m_columnsAssociation.clear();
    m_map.clear();
}

index_t TableMap::size() const {
    return m_rows.size();
}

bool TableMap::empty() const {
    return m_rows.empty();
}

}  // namespace table
}  // namespace octf
