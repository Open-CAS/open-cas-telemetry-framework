/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H
#define SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H

#include <memory>

namespace octf {

/**
 * @ingroup Containers
 * @brief Basic const iterator interface
 *
 * This interface defines required methods for const iterator which allows to
 * iterate over const container
 *
 * @tparam type Type of container's element
 * @tparam const_iterator Type of container's const iterator
 * @note const_iterator must inherits by IIteratorConst
 * @interface IIteratorConst
 */
template <typename type, typename const_iterator>
class IIteratorConst {
public:
    IIteratorConst() = default;
    virtual ~IIteratorConst() = default;

    /**
     * @brief Const reference operator to container's element at current const
     * iterator position
     *
     * @return Const reference to container's element at current const iterator
     */
    virtual const type &operator*() const = 0;

    /**
     * @brief Const pointer operator to container's element at current const
     * iterator position
     *
     * @return Const pointer to container's element at current const iterator
     */
    virtual const type *operator->() const = 0;

    /**
     * @brief pre-increment const iterator operator
     *
     * @return Iterator at next position
     */
    virtual const_iterator &operator++() = 0;

    /**
     * @brief Post-increment const iterator operator
     *
     * Returns const iterator at current position and increments to next one
     *
     * @return Iterator at current position
     */
    virtual const_iterator operator++(int) = 0;

    /**
     * @brief Pre-decrement const iterator operator
     *
     * @return Iterator at previous position
     */
    virtual const_iterator &operator--() = 0;

    /**
     * @brief Post-decrement const iterator operator
     *
     * Returns const iterator at current position and decrement to previous one
     *
     * @return Iterator at current position
     */
    virtual const_iterator operator--(int) = 0;

    /**
     * @brief Compares const iterators operator
     *
     * @param other Other const iterator to be compared
     *
     * @retval true Operators are equal
     * @retval false Operators are not equal
     */
    virtual bool operator==(const const_iterator &other) const = 0;

    /**
     * @brief Checks if const iterators are different
     *
     * @param other Other const iterator to be checked
     *
     * @retval true Operators are different
     * @retval false Operators are not different
     */
    virtual bool operator!=(const const_iterator &other) const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H
