/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_IITERATOR_H
#define SOURCE_OCTF_UTILS_CONTAINER_IITERATOR_H

#include <memory>

namespace octf {

/**
 * @ingroup Containers
 * @brief Basic iterator interface
 *
 * This interface defines required methods for const iterator which allows to
 * iterate over container
 *
 * @tparam type Type of container's element
 * @tparam const iterator Type of container's iterator
 * @note iterator must inherits by IIterator
 * @interface IIterator
 */
template <typename type, typename iterator>
class IIterator {
public:
    IIterator() = default;
    virtual ~IIterator() = default;

    /**
     * @brief Reference operator to container's element at current iterator
     * position
     *
     * @return Reference to container's element at current iterator
     */
    virtual type &operator*() const = 0;

    /**
     * @brief Pointer operator to container's element at current iterator
     * position
     *
     * @return Pointer to container's element at current iterator
     */
    virtual type *operator->() const = 0;

    /**
     * @brief Pre-increment iterator operator
     *
     * @return Iterator at next position
     */
    virtual iterator &operator++() = 0;

    /**
     * @brief Post-increment iterator operator
     *
     * Returns iterator at current position and increments to next one
     *
     * @return Iterator at current position
     */
    virtual iterator operator++(int) = 0;

    /**
     * @brief Pre-decrement iterator operator
     *
     * @return Iterator at previous position
     */
    virtual iterator &operator--() = 0;

    /**
     * @brief Post-decrement iterator operator
     *
     * Returns iterator at current position and decrement to previous one
     *
     * @return Iterator at current position
     */
    virtual iterator operator--(int) = 0;

    /**
     * @brief Compares iterators operator
     *
     * @param other Other iterator to be compared
     *
     * @retval true Operators are equal
     * @retval false Operators are not equal
     */
    virtual bool operator==(const iterator &other) const = 0;

    /**
     * @brief Checks if iterators are different
     *
     * @param other Other iterator to be checked
     *
     * @retval true Operators are different
     * @retval false Operators are not differnet
     */
    virtual bool operator!=(const iterator &other) const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_IITERATOR_H
