/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACESERVICEIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACESERVICEIMPL_H

#include <google/protobuf/service.h>
#include <octf/node/INode.h>
#include <octf/proto/InterfaceService.pb.h>

namespace octf {

class InterfaceServiceImpl : public proto::InterfaceService {
public:
    InterfaceServiceImpl(INode *node)
            : m_owner(node){};
    virtual ~InterfaceServiceImpl() = default;
    void registerPlugin(::google::protobuf::RpcController *controller,
                        const ::octf::proto::NodeId *request,
                        ::octf::proto::Void *response,
                        ::google::protobuf::Closure *done) override;
    void unregisterPlugin(::google::protobuf::RpcController *controller,
                          const ::octf::proto::NodeId *request,
                          ::octf::proto::Void *response,
                          ::google::protobuf::Closure *done) override;
    void connectPlugin(::google::protobuf::RpcController *controller,
                       const ::octf::proto::NodeId *request,
                       ::octf::proto::Void *response,
                       ::google::protobuf::Closure *done) override;
    void disconnectPlugin(::google::protobuf::RpcController *controller,
                          const ::octf::proto::NodeId *request,
                          ::octf::proto::Void *response,
                          ::google::protobuf::Closure *done) override;
    void listPlugins(::google::protobuf::RpcController *controller,
                     const ::octf::proto::Void *request,
                     ::octf::proto::PluginsList *response,
                     ::google::protobuf::Closure *done) override;

private:
    INode *m_owner;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACESERVICEIMPL_H
