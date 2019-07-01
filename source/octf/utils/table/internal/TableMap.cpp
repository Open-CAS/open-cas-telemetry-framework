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

size_t TableMap::getRowAssociation(const std::string &name) {
    size_t index = m_rowsAssociation.size();
    auto iter = m_rowsAssociation.find(name);

    if (iter != m_rowsAssociation.end()) {
        index = iter->second;
    } else {
        m_rowsAssociation[name] = index;
    }

    return index;
}

size_t TableMap::getColumnAssociation(const std::string &name) {
    size_t index = m_columnsAssociation.size();
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
        getColumn(addr.getColumn());

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

Row &TableMap::getRow(size_t index) {
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

const Row &TableMap::getRow(size_t index) const {
    auto iter = m_rows.find(index);

    if (iter != m_rows.end()) {
        throw Exception("Accessing non-existing row in table");
    } else {
        return iter->second;
    }
}

Column &TableMap::getColumn(size_t index) {
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

CellIterator TableMap::beginCell(size_t id) {
    return CellIterator(*this, m_columns.begin(), id);
}

CellIterator TableMap::endCell(size_t id) {
    return CellIterator(*this, m_columns.end(), id);
}

CellIterator octf::table::TableMap::eraseCell(CellIterator iter) {
    if (iter.m_link != m_columns.end()) {
        Addr addr(iter.m_id, iter.m_link->first);
        m_map.erase(addr);

        iter.m_link++;
        if (iter.m_link == m_columns.end()) {
            // Last cell removed, cleanup this row entirely

            m_rows.erase(iter.m_id);
        }
    }

    return iter;
}

CellIteratorConst TableMap::beginCell(size_t id) const {
    return CellIteratorConst(*this, m_columns.begin(), id);
}

CellIteratorConst TableMap::endCell(size_t id) const {
    return CellIteratorConst(*this, m_columns.end(), id);
}

size_t TableMap::getRowCount() const {
    return m_rows.size();
}

size_t TableMap::getColumnCount() const {
    return m_rows.size();
}

void octf::table::TableMap::clear() {
    m_rows.clear();
    m_rowsAssociation.clear();
    m_columns.clear();
    m_columnsAssociation.clear();
    m_map.clear();
}

size_t TableMap::size() const {
    return m_rows.size();
}

bool TableMap::empty() const {
    return m_rows.empty();
}

}  // namespace table
}  // namespace octf
