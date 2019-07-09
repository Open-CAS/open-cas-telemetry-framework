/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Column.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Column::Column(index_t id, TableMap &map)
        : Set(map)
        , m_columnIndex(id)
        , m_map(map) {}

Column::~Column() {}

Addr Column::getAddress(index_t index) {
    Addr addr(index, m_columnIndex);
    return addr;
}

Addr Column::getAddress(const std::string &index) {
    Addr addr(m_map.getRowAssociation(index), m_columnIndex);
    return addr;
}

}  // namespace table
}  // namespace octf
