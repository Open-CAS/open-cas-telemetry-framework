/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/octf.h>
#include "TestPlugin.h"

using namespace octf;

int main() {
    try {
        TestPlugin plugin;
        if (!plugin.init()) {
            return false;
        }

        // Wait for termination signal
        SignalHandler::get().registerSignal(SIGTERM);
        SignalHandler::get().registerSignal(SIGINT);
        SignalHandler::get().wait();
        SignalHandler::get().clearAllSignals();

        plugin.deinit();
        return true;
    } catch (Exception &ex) {
        log::critical << ex.what() << std::endl;
        return 1;
    }
}
