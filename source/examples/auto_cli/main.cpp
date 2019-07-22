/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <examples/auto_cli/InterfaceHelloWorldImpl.h>
#include <memory>
#include <string>
#include <octf/octf.h>

int main(int argc, char *argv[]) {
    // Create executor and local command set
    octf::cli::Executor ex;

    // Configure CLI program
    auto &properties = ex.getCliProperties();
    properties.setName("auto-cli");
    properties.setVersion("1.0");

    // Create interface
    octf::InterfaceShRef iHelloWord =
            std::make_shared<InterfaceHelloWorldImpl>();

    // Add interface to executor
    ex.addModules(iHelloWord);

    // Execute command
    return ex.execute(argc, argv);
}
