/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ISET_H
#define SOURCE_OCTF_UTILS_TABLE_ISET_H

#include <stddef.h>
#include <string>
#include <octf/utils/NonCopyable.h>
#include <octf/utils/table/Addr.h>

namespace octf {
namespace table {

class Cell;

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
    virtual Cell &operator[](size_t index) = 0;

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
    virtual Addr getAddress(size_t index) = 0;

    /**
     * @brief Get cell address by cell name
     * @param index Cell name
     * @return Cell address
     */
    virtual Addr getAddress(const std::string &index) = 0;

    /**
     * @brief Gets id of set
     * @return Id
     */
    virtual size_t getId() const = 0;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ISET_H
