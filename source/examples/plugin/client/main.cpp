/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/octf.h>
#include "TestPluginShadow.h"

using namespace octf;

int main(void) {
    TestPluginShadow plugin;

    if (!plugin.init()) {
        // Cannot initialize plugin
        return -1;
    }

    // Call a method from a plugin interface
    Call<proto::Void, proto::NodeId> call(&plugin);
    plugin.getIdentificationInterface()->getNodeId(
            &call, call.getInput().get(), call.getOutput().get(), &call);

    std::chrono::milliseconds timeout(100);
    call.waitFor(timeout);

    if (call.Failed()) {
        // Method call failed
        log::cerr << call.ErrorText() << std::endl;

        return -1;
    } else {
        log::cout << "Node ID: " << call.getOutput()->id() << std::endl;

        return 0;
    }
}
