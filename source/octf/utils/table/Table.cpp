/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Table.h>

#include <sstream>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Table::Table()
        : NonCopyable()
        , m_map(new TableMap()) {}

Table::~Table() {}

Row &Table::operator[](index_t row) {
    return m_map->getRow(row);
}

Row &Table::operator[](const std::string &row) {
    index_t index = m_map->getRowAssociation(row);
    return (*this)[index];
}

const Row &Table::operator[](index_t row) const {
    auto const &map = *m_map;

    return map.getRow(row);
}

const Row &Table::operator[](const std::string &row) const {
    index_t index = m_map->getRowAssociation(row);
    return (*this)[index];
}

RowIterator Table::begin() {
    return m_map->beginRow();
}

RowIterator Table::end() {
    return m_map->endRow();
}

RowIteratorConst Table::begin() const {
    auto const &map = *m_map;

    return map.beginRow();
}

RowIteratorConst Table::end() const {
    auto const &map = *m_map;

    return map.endRow();
}

void Table::clear() {
    m_map->clear();
}

index_t Table::size() const {
    auto const &map = *m_map;
    return map.size();
}

bool Table::empty() const {
    auto const &map = *m_map;
    return map.empty();
}

}  // namespace table
}  // namespace octf
