/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/internal/Configuration.h>

#include <mutex>
#include <octf/utils/Exception.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {
namespace proto {

FrameworkConfiguration &getFrameworkConfiguration() {
    static std::mutex mutex;
    static FrameworkConfiguration config;
    static bool loaded = false;

    if (!loaded) {
        std::lock_guard<std::mutex> lock(mutex);

        if (!loaded) {
            ProtobufReaderWriter rw(OCTF_CONFIG_FILE);

            if (false == rw.read(config)) {
                throw Exception("Cannot read framework configuration");
            }

            loaded = true;
        }
    }

    return config;
}

void saveFrameworkConfiguration() {
    ProtobufReaderWriter rw(OCTF_CONFIG_FILE);

    if (false == rw.write(getFrameworkConfiguration())) {
        throw Exception("Cannot read framework configuration");
    }
}

}  // namespace proto
}  // namespace octf
