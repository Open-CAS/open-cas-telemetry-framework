/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACECONFIGURATIONIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACECONFIGURATIONIMPL_H

#include <octf/node/INode.h>
#include <octf/proto/InterfaceConfiguration.pb.h>

namespace octf {

/**
 * @brief It allows to configure the OCTF framework
 */
class InterfaceConfigurationImpl : public proto::InterfaceConfiguration {
public:
    InterfaceConfigurationImpl() = default;
    virtual ~InterfaceConfigurationImpl() = default;

    void getTraceRepositoryPath(::google::protobuf::RpcController *controller,
                                const ::octf::proto::Void *request,
                                ::octf::proto::TraceRepositoryPath *response,
                                ::google::protobuf::Closure *done) override;

    void setTraceRepositoryPath(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::TraceRepositoryPath *request,
            ::octf::proto::Void *response,
            ::google::protobuf::Closure *done) override;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACECONFIGURATIONIMPL_H
