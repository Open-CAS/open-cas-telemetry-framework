/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H

#include <memory>

#include <octf/node/INode.h>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/trace.pb.h>

namespace octf {

class InterfaceTraceParsingImpl : public proto::InterfaceTraceParsing {
public:
    InterfaceTraceParsingImpl() = default;
    virtual ~InterfaceTraceParsingImpl() = default;

    virtual void ParseTrace(::google::protobuf::RpcController *controller,
                            const ::octf::proto::ParseTraceRequest *request,
                            ::octf::proto::Void *response,
                            ::google::protobuf::Closure *done) override;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H
