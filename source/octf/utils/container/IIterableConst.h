/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_IITERABLECONST_H
#define SOURCE_OCTF_UTILS_CONTAINER_IITERABLECONST_H

namespace octf {

/**
 * @ingroup Containers
 * @brief Basic const iterable interface
 *
 * It allows to iterate through const container from the beginning to the end
 *
 * @tparam const_iterator Type of const containers iterator
 * @note const_iterator must inherits by IIteratorConst
 * @interface IIterableConst
 */
template <typename const_iterator>
class IIterableConst {
public:
    IIterableConst() = default;
    virtual ~IIterableConst() = default;

    /**
     * @briefs Gets begin position const iterator
     *
     * @return Begin position const iterator
     */
    virtual const_iterator begin() const = 0;

    /**
     * @briefs Gets end position const iterator
     *
     * @return End position const iterator
     */
    virtual const_iterator end() const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_IITERABLECONST_H
