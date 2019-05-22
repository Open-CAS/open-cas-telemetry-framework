/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "InterfaceTestImpl.h"

namespace octf {

void InterfaceTestImpl::HelloWorld(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::HelloWorldRequest *request,
        ::octf::proto::HelloWorldResponse *response,
        ::google::protobuf::Closure *done) {
    // Simple method, just return result, controller not used
    (void) controller;

    std::string s = "Test response to: " + request->hello();
    response->set_helloresp(s);
    done->Run();
}

}  // namespace octf
