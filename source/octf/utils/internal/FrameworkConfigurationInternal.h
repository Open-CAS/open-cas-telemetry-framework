/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_INTERNAL_FRAMEWORKCONFIGURATIONINTERNAL_H
#define SOURCE_OCTF_UTILS_INTERNAL_FRAMEWORKCONFIGURATIONINTERNAL_H

/**
 * @file FrameworkConfigurationInternal.h
 *
 * @brief The internal API for getting and setting the configuration of
 * Framework
 */

#include <string>
#include <octf/proto/configuration.pb.h>

#ifndef OCTF_CONFIG_FILE
#error "OCTF_CONFIG_FILE not defined"
#endif

namespace octf {
namespace proto {

/**
 * @brief Gets the framework configuration
 *
 * It returns the framework configuration and upon first call, it loads
 * configuration from the file
 *
 * @return Framework Configuration
 */
proto::FrameworkConfiguration &getFrameworkConfiguration();

/**
 * @brief Persists the framework configuration
 */
void saveFrameworkConfiguration();

}  // namespace proto
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_INTERNAL_FRAMEWORKCONFIGURATIONINTERNAL_H
