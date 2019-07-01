/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ROW_H
#define SOURCE_OCTF_UTILS_TABLE_ROW_H

#include <stddef.h>
#include <map>
#include <octf/utils/container/IContainer.h>
#include <octf/utils/table/Iterators.h>
#include <octf/utils/table/Set.h>

namespace octf {
namespace table {

/**
 * @ingroup Table
 *
 * @brief Table Row
 */
class Row : public Set,
            public IContainer<Cell, CellIterator, CellIteratorConst> {
public:
    Row(size_t id, TableMap &map);
    virtual ~Row();

    Addr getAddress(size_t index) override;

    Addr getAddress(const std::string &index) override;

    size_t getId() const override {
        return m_id;
    }

    CellIterator begin() override;

    CellIterator end() override;

    CellIteratorConst begin() const override;

    CellIteratorConst end() const override;

    void clear() override;

    size_t size() const override;

    bool empty() const override;

private:
    const size_t m_id;
    TableMap &m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ROW_H
