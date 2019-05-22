/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/DateTime.h>

#include <time.h>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <octf/utils/Exception.h>

namespace octf {
namespace datetime {

static constexpr size_t MAX_TIME_STRING_BUFFER = 255;

using namespace std::chrono;

std::string getFormattedDateTime(
        const std::chrono::time_point<std::chrono::steady_clock> &timePoint,
        const std::string &format) {
    using namespace std::chrono;

    // Create time_t from timepoint
    time_t time = system_clock::to_time_t(system_clock::now() +
                                          (timePoint - steady_clock::now()));

    // Create a type which recognizes local time with respect to civil calendar
    tm dateTime = *localtime(&time);

    // Buffer for result
    char buffer[MAX_TIME_STRING_BUFFER + 1];
    buffer[MAX_TIME_STRING_BUFFER] = '\0';

    // Format date time
    if (!strftime(buffer, MAX_TIME_STRING_BUFFER, format.c_str(), &dateTime)) {
        throw Exception("Error converting time.");
    }

    return std::string(buffer);
}

std::string getTimestamp() {
    const auto now = steady_clock::now();
    const auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream result;

    result << getFormattedDateTime(now);
    result << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return result.str();
}

}  // namespace datetime
}  // namespace octf
