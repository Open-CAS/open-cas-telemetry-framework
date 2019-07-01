/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_SET_H
#define SOURCE_OCTF_UTILS_TABLE_SET_H

#include <map>
#include <memory>
#include <octf/utils/container/IIterator.h>
#include <octf/utils/table/Cell.h>
#include <octf/utils/table/ISet.h>

namespace octf {
namespace table {

class TableMap;

class Set : public ISet {
public:
    Set(TableMap &map);
    virtual ~Set();

    Cell &operator[](size_t index) override;

    Cell &operator[](const std::string &index) override;

private:
    TableMap &m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_SET_H
