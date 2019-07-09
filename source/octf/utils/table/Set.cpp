/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Set.h>

#include <octf/utils/Exception.h>
#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Set::Set(TableMap &map)
        : ISet()
        , m_map(map) {}

Set::~Set() {}

Cell &Set::operator[](index_t index) {
    return m_map[getAddress(index)];
}

Cell &Set::operator[](const std::string &index) {
    return m_map[getAddress(index)];
}

}  // namespace table
}  // namespace octf
