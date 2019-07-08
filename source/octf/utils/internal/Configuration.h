/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_INTERNAL_CONFIGURATION_H
#define SOURCE_OCTF_UTILS_INTERNAL_CONFIGURATION_H

#include <string>
#include <octf/proto/configuration.pb.h>

#ifndef OCTF_CONFIG_FILE
#error "OCTF_CONFIG_FILE not defined"
#endif

namespace octf {
namespace proto {

proto::FrameworkConfiguration &getFrameworkConfiguration();
void saveFrameworkConfiguration();

}  // namespace proto
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_INTERNAL_CONFIGURATION_H
