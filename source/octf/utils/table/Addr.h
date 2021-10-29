/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ADDR_H
#define SOURCE_OCTF_UTILS_TABLE_ADDR_H

#include <stddef.h>
#include <octf/utils/table/Types.h>

namespace octf {
namespace table {

/**
 * @ingroup Table
 * @brief Cell address
 */
class Addr {
public:
    Addr()
            : m_row(0)
            , m_column(0) {}

    Addr(index_t row, index_t column)
            : m_row(row)
            , m_column(column) {}

    Addr(const Addr &other)
            : m_row(other.m_row)
            , m_column(other.m_column) {}

    virtual ~Addr() = default;

    Addr &operator=(const Addr &other) {
        if (this != &other) {
            m_row = other.m_row;
            m_column = other.m_column;
        }

        return *this;
    }

    bool operator=(const Addr &other) const {
        return (m_row == other.m_row) && (m_column == other.m_column);
    }

    bool operator<(const Addr &other) const {
        if (m_row == other.m_row) {
            return m_column < other.m_column;
        }

        return m_row < other.m_row;
    }

    /**
     * @brief Gets column index
     * @return Column index
     */
    index_t getColumnIndex() const {
        return m_column;
    }

    /**
     * @brief Gets row index
     * @return Row index
     */
    index_t getRowIndex() const {
        return m_row;
    }

private:
    index_t m_row;
    index_t m_column;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ADDR_H
