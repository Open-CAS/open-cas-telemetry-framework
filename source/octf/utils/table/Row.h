/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ROW_H
#define SOURCE_OCTF_UTILS_TABLE_ROW_H

#include <stddef.h>
#include <map>
#include <octf/utils/container/IContainer.h>
#include <octf/utils/table/Iterators.h>
#include <octf/utils/table/Set.h>
#include <octf/utils/table/Types.h>

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
    Row(index_t id, TableMap &map);
    virtual ~Row();

    void setupHeader();

    Addr getAddress(index_t columnIndex) override;

    Addr getAddress(const std::string &columnIndex) override;

    index_t getIndex() const override {
        return m_rowIndex;
    }

    CellIterator begin() override;

    CellIterator end() override;

    CellIteratorConst begin() const override;

    CellIteratorConst end() const override;

    void clear() override;

    index_t size() const override;

    bool empty() const override;

private:
    const index_t m_rowIndex;
    TableMap &m_map;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ROW_H
