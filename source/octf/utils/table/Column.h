/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_COLUMN_H
#define SOURCE_OCTF_UTILS_TABLE_COLUMN_H

#include <stddef.h>
#include <octf/utils/table/Set.h>

namespace octf {
namespace table {

/**
 * @ingroup Table
 * @brief Table column
 */
class Column : public Set {
public:
    Column(size_t id, TableMap &map);
    virtual ~Column();

    Addr getAddress(size_t index) override;

    Addr getAddress(const std::string &index) override;

    size_t getId() const override {
        return m_id;
    }

private:
    const size_t m_id;
    TableMap &m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_COLUMN_H
