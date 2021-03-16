/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Row.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Row::Row(index_t id, TableMap &map)
        : Set(map)
        , m_rowIndex(id)
        , m_map(map) {}

Row::~Row() {}

Addr Row::getAddress(index_t columnIndex) {
    Addr addr(m_rowIndex, columnIndex);
    return addr;
}

Addr Row::getAddress(const std::string &columnIndex) {
    Addr addr(m_rowIndex, m_map.getColumnAssociation(columnIndex));
    return addr;
}

CellIterator Row::begin() {
    return m_map.beginCell(getIndex());
}

CellIterator Row::end() {
    return m_map.endCell(getIndex());
}

CellIteratorConst Row::begin() const {
    const auto &map = m_map;
    return map.beginCell(getIndex());
}

CellIteratorConst Row::end() const {
    const auto &map = m_map;
    return map.endCell(getIndex());
}

void Row::clear() {
    auto iter = m_map.beginCell(getIndex());
    auto end = m_map.endCell(getIndex());
    while (iter != end) {
        iter = m_map.eraseCell(iter);
    }
}

index_t Row::size() const {
    return m_map.getColumnCount();
}

bool Row::empty() const {
    return 0 == m_map.getColumnCount();
}

void octf::table::Row::setupHeader() {
    for (const auto &association : m_map.getColumnsAssociation()) {
        const auto &name = association.first;
        (*this)[name] = name;
    }
}

}  // namespace table
}  // namespace octf
