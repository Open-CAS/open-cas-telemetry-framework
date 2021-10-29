/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Exception.h"

namespace octf {

Exception::Exception(const std::string &message)
        : m_message(message) {}

const char *Exception::what() const noexcept {
    return m_message.c_str();
}

const std::string &Exception::getMessage() const {
    return m_message;
}
}  // namespace octf
