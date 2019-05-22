/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_DATETIME_H
#define SOURCE_OCTF_UTILS_DATETIME_H

#include <chrono>
#include <string>

namespace octf {
namespace datetime {

/**
 * @brief Gets string date time for specified time point and format
 *
 * @param timePoint Time point
 * @param format Date time format
 *
 * @return Data time
 */
std::string getFormattedDateTime(
        const std::chrono::time_point<std::chrono::steady_clock> &timePoint,
        const std::string &format = "%Y-%m-%d %H:%M");

/**
 * @brief Gets string time stamp
 *
 * In addition the returned time stamp will contain milliseconds
 *
 * @return string time stamp
 */
std::string getTimestamp();

}  // namespace datetime
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_DATETIME_H
