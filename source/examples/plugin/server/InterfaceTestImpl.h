/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_EXAMPLES_PLUGIN_SERVER_INTERFACETESTIMPL_H
#define SOURCE_EXAMPLES_PLUGIN_SERVER_INTERFACETESTIMPL_H

#include "InterfaceTest.pb.h"

namespace octf {

class InterfaceTestImpl : public proto::InterfaceTest {
public:
    InterfaceTestImpl() = default;
    virtual ~InterfaceTestImpl() = default;

    virtual void HelloWorld(::google::protobuf::RpcController *controller,
                            const ::octf::proto::HelloWorldRequest *request,
                            ::octf::proto::HelloWorldResponse *response,
                            ::google::protobuf::Closure *done) override;
};

}  // namespace octf

#endif  // SOURCE_EXAMPLES_PLUGIN_SERVER_INTERFACETESTIMPL_H
