/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ISET_H
#define SOURCE_OCTF_UTILS_TABLE_ISET_H

#include <stddef.h>
#include <string>
#include <octf/utils/table/Addr.h>
#include <octf/utils/table/Types.h>

namespace octf {
namespace table {

/**
 * @ingroup Table
 *
 * @brief Set of cells
 *
 * @interface ISet
 */
class ISet {
public:
    ISet() = default;
    virtual ~ISet() = default;

    /**
     * @brief Gets cell by index
     * @param index Cell index
     * @return Cell
     */
    virtual Cell &operator[](index_t index) = 0;

    /**
     * @brief Get cell by name
     * @param index Cell name
     * @return Cell
     */
    virtual Cell &operator[](const std::string &index) = 0;

    /**
     * @brief Gets cell address by cell index
     * @param index Cell index
     * @return Cell address
     */
    virtual Addr getAddress(index_t index) = 0;

    /**
     * @brief Get cell address by cell name
     * @param index Cell name
     * @return Cell address
     */
    virtual Addr getAddress(const std::string &index) = 0;

    /**
     * @brief Gets this set index
     * @return Id
     */
    virtual index_t getIndex() const = 0;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ISET_H
