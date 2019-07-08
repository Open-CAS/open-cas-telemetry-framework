/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/Exception.h>
#include <octf/utils/ProtobufReaderWriter.h>
#include <octf/utils/internal/Configuration.h>

namespace octf {
namespace proto {

FrameworkConfiguration &getFrameworkConfiguration() {
    static FrameworkConfiguration config;
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
