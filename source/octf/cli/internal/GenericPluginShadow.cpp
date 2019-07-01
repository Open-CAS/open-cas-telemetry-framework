/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/GenericPluginShadow.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>

namespace octf {
namespace cli {

GenericPluginShadow::GenericPluginShadow(const std::string &name)
        : NodePluginShadow(name) {}

GenericPluginShadow::~GenericPluginShadow(){};

bool GenericPluginShadow::initCustom() {
    if (!createInterface<proto::InterfaceCli_Stub>()) {
        throw Exception("Error creating CLI interface");
    }

    return true;
}

void GenericPluginShadow::deinitCustom() {}

}  // namespace cli
}  // namespace octf
