/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Column.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Column::Column(size_t id, TableMap &map)
        : Set(map)
        , m_id(id)
        , m_map(map) {}

Column::~Column() {}

Addr Column::getAddress(size_t index) {
    Addr addr(index, m_id);
    return addr;
}

Addr Column::getAddress(const std::string &index) {
    Addr addr(m_map.getRowAssociation(index), m_id);
    return addr;
}

}  // namespace table
}  // namespace octf
