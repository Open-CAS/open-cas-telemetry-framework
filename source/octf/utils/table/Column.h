/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_COLUMN_H
#define SOURCE_OCTF_UTILS_TABLE_COLUMN_H

#include <stddef.h>
#include <octf/utils/table/Set.h>
#include <octf/utils/table/Types.h>

namespace octf {
namespace table {

/**
 * @ingroup Table
 * @brief Table column
 */
class Column : public Set {
public:
    Column(index_t id, TableMap &map);
    virtual ~Column();

    Addr getAddress(index_t rowIndex) override;

    Addr getAddress(const std::string &rowIndex) override;

    index_t getIndex() const override {
        return m_columnIndex;
    }

private:
    const index_t m_columnIndex;
    TableMap &m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_COLUMN_H
