/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "TestPlugin.h"

#include "InterfaceTestImpl.h"
#include "test.pb.h"

namespace octf {

TestPlugin::TestPlugin()
        : NodePlugin(NodeId("test")) {}

TestPlugin::~TestPlugin() {}

bool TestPlugin::initCustom() {
    if (!initSettings<proto::test::Settings>()) {
        return false;
    }

    if (!createInterface<InterfaceTestImpl>()) {
        throw Exception("Error creating plugin test interface");
    }

    if (!createInterface<InterfaceCliImpl>(this)) {
        throw Exception("Error creating plugin CLI interface");
    }

    return true;
}

void TestPlugin::deinitCustom() {}

}  // namespace octf
