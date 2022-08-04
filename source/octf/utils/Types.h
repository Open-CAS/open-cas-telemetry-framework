/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_TYPES_H
#define SOURCE_OCTF_UTILS_TYPES_H

#include <google/protobuf/message.h>

namespace octf {

/**
 * @brief Share reference to protocol buffer message
 */
typedef std::shared_ptr<google::protobuf::Message> MessageShRef;

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TYPES_H
