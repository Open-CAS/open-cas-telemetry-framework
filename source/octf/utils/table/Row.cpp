/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Row.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Row::Row(size_t id, TableMap &map)
        : Set(map)
        , m_id(id)
        , m_map(map) {}

Row::~Row() {}

Addr Row::getAddress(size_t index) {
    Addr addr(m_id, index);
    return addr;
}

Addr Row::getAddress(const std::string &index) {
    Addr addr(m_id, m_map.getColumnAssociation(index));
    return addr;
}

CellIterator Row::begin() {
    return m_map.beginCell(getId());
}

CellIterator Row::end() {
    return m_map.endCell(getId());
}

CellIteratorConst Row::begin() const {
    const auto &map = m_map;
    return map.beginCell(getId());
}

CellIteratorConst Row::end() const {
    const auto &map = m_map;
    return map.endCell(getId());
}

void Row::clear() {
    auto iter = m_map.beginCell(getId());
    while (iter != m_map.endCell(getId())) {
        iter = m_map.eraseCell(iter);
    }
}

size_t Row::size() const {
    return m_map.getColumnCount();
}

bool Row::empty() const {
    return 0 == m_map.getColumnCount();
}

}  // namespace table
}  // namespace octf
