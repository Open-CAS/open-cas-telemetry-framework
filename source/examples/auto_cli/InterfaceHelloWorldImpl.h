/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_EXAMPLES_CLI_INTERFACEHELLOWORLDIMPL_H
#define SOURCE_EXAMPLES_CLI_INTERFACEHELLOWORLDIMPL_H

#include "InterfaceHelloWorld.pb.h"

class InterfaceHelloWorldImpl : public InterfaceHelloWorld {
public:
    InterfaceHelloWorldImpl() = default;
    virtual ~InterfaceHelloWorldImpl() = default;

    virtual void HelloWorld(::google::protobuf::RpcController *controller,
                            const ::Request *request,
                            ::Response *response,
                            ::google::protobuf::Closure *done) override;
};

#endif  // SOURCE_EXAMPLES_CLI_INTERFACEHELLOWORLDIMPL_H
