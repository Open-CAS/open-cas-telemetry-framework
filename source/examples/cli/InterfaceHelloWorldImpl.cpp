/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "InterfaceHelloWorldImpl.h"

#include <string>
#include <octf/octf.h>

void InterfaceHelloWorldImpl::HelloWorld(
        ::google::protobuf::RpcController *controller,
        const ::Request *request,
        ::Response *response,
        ::google::protobuf::Closure *done) {
    if ("" == request->hello()) {
        // Terminate this method with the error
        controller->SetFailed("Error. You have to say something");
        done->Run();
        return;
    }

    if ("magic" == request->hello()) {
        // Or just throw exception in order to report command execution error
        throw octf::Exception("I don't like magic");
    }

    // Prepare response
    std::string value = "Hi, this is CLI. You typed: ";
    value += request->hello();

    // Set response
    response->set_helloresponse(value);

    // End method
    done->Run();
}
