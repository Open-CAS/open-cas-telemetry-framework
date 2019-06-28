/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_IITERABLE_H
#define SOURCE_OCTF_UTILS_CONTAINER_IITERABLE_H

namespace octf {

/**
 * @ingroup Containers
 * @brief Basic iterable interface
 *
 * It allows to iterate through container from the beginning to the end
 *
 * @tparam iterator Type of containers iterator
 * @note iterator must inherits by IIterator
 * @interface IIterable
 */
template <typename iterator>
class IIterable {
public:
    IIterable() = default;
    virtual ~IIterable() = default;

    /**
     * @briefs Gets begin position iterator
     *
     * @return Begin position iterator
     */
    virtual iterator begin() = 0;

    /**
     * @briefs Gets end position iterator
     *
     * @return End position iterator
     */
    virtual iterator end() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_IITERABLE_H
