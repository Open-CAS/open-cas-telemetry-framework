/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "TestPluginShadow.h"

namespace octf {
TestPluginShadow::TestPluginShadow()
        : NodePluginShadow(NodeId("test")) {}

TestPluginShadow::~TestPluginShadow(){};

bool TestPluginShadow::initCustom() {
    if (!createInterface<proto::InterfaceCli_Stub>()) {
        throw Exception("Error creating CLI interface");
    }

    return true;
}

}  // namespace octf
