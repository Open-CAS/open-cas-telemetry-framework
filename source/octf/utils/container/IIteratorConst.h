/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H
#define SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H

#include <memory>

namespace octf {

/**
 * @ingroup Containers
 * @brief Basic const iterator interface
 *
 * This interface defines required methods for iterator which allows to iterate
 * over const container
 *
 * @tparam type Type of container's element
 * @tparam iterator Type of container's const iterator
 * @note iterator must inherits by IIteratorConst
 * @interface IIteratorConst
 */
template <typename type, typename const_iterator>
class IIteratorConst {
public:
    IIteratorConst() = default;
    virtual ~IIteratorConst() = default;

    /**
     * @brief Const reference operator to contaier's element at current iterator
     * position
     *
     * @return Const reference to contaier's element at current iterator
     */
    virtual const type &operator*() const = 0;

    /**
     * @brief Const pointer operator to contaier's element at current iterator
     * position
     *
     * @return Const pointer to contaier's element at current iterator
     */
    virtual const type *operator->() const = 0;

    /**
     * @brief Post-increment iterator operator
     *
     * @return Iterator at next position
     */
    virtual const_iterator &operator++() = 0;

    /**
     * @brief Pre-increment iterator operator
     *
     * Returns iterator at current position and increments to next one
     *
     * @return Iterator at current position
     */
    virtual const_iterator operator++(int) = 0;

    /**
     * @brief Post-decrement iterator operator
     *
     * @return Iterator at previous position
     */
    virtual const_iterator &operator--() = 0;

    /**
     * @brief Pre-decrement iterator operator
     *
     * Returns iterator at current position and decrement to previous one
     *
     * @return Iterator at current position
     */
    virtual const_iterator operator--(int) = 0;

    /**
     * @brief Compares iterators operator
     *
     * @param other Other iterator to be compared
     *
     * @retval true Operators are equal
     * @retval false Operators are not equal
     */
    virtual bool operator==(const const_iterator &other) const = 0;

    /**
     * @brief Checks if iterators are different
     *
     * @param other Other iterator to be checked
     *
     * @retval true Operators are different
     * @retval false Operators are not differnet
     */
    virtual bool operator!=(const const_iterator &other) const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_IITERATORCONST_H
