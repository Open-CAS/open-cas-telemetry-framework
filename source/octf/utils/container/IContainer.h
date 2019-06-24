/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_CONTAINER_ICONTAINER_H
#define SOURCE_OCTF_UTILS_CONTAINER_ICONTAINER_H

#include <octf/utils/container/IIterable.h>
#include <octf/utils/container/IIterableConst.h>

namespace octf {

/**
 * @defgroup Containers Containers
 * Container interface specification to implement a container with iteration
 * capabilities
 *
 * @ingroup Utilities
 */

/**
 * @ingroup Containers
 * @brief Basic container interface
 *
 * @tparam type Type of container's element
 * @tparam iterator Type of containers iterator
 * @tparam const_iterator Type of containers const iterator
 *
 * @note iterator must inherits by IIterator
 * @note const_iterator must inherits by IIteratorConst
 *
 * @interface IContainer
 */
template <typename type, typename iterator, typename const_iterator>
class IContainer : public IIterable<iterator>,
                   public IIterableConst<const_iterator> {
public:
    static_assert(std::is_base_of<IIterator<type, iterator>, iterator>(),
                  "iterator must inherit by IIterator");

    static_assert(std::is_base_of<IIteratorConst<type, const_iterator>,
                                  const_iterator>(),
                  "iterator must inherit by IIterator");

    IContainer() = default;
    virtual ~IContainer() = default;

    /**
     * @brief Clears content of container
     */
    virtual void clear() = 0;

    /**
     * @brief Returns number of elements in container
     *
     * @return Number of elements in container
     */
    virtual size_t size() const = 0;

    /**
     * @brief Indicates if container is empty
     *
     * @retval true Container is empty
     * @retval false Container is not empty
     */
    virtual bool empty() const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_CONTAINER_ICONTAINER_H
