/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_NONCOPYABLE_H
#define SOURCE_OCTF_UTILS_NONCOPYABLE_H

namespace octf {

/**
 * @brief Class which protects against calling coping constructor and assign
 * operator
 *
 * To enable no-copy functionality just use private inheritance by this class
 */
class NonCopyable {
public:
    NonCopyable(NonCopyable const &) = delete;
    NonCopyable &operator=(NonCopyable const &) = delete;
    NonCopyable() {}
    virtual ~NonCopyable() {}
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_NONCOPYABLE_H
