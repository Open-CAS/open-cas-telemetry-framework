/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACEIDENTIFICATIONIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACEIDENTIFICATIONIMPL_H

#include <google/protobuf/service.h>
#include <octf/node/INode.h>
#include <octf/proto/InterfaceIdentification.pb.h>

namespace octf {

class InterfaceIdentificationImpl : public proto::InterfaceIdentification {
public:
    InterfaceIdentificationImpl(INode *node)
            : m_owner(node){};

    virtual ~InterfaceIdentificationImpl() = default;

    void getNodeId(::google::protobuf::RpcController *controller,
                   const ::octf::proto::Void *request,
                   ::octf::proto::NodeId *response,
                   ::google::protobuf::Closure *done) override;

    void getChildren(::google::protobuf::RpcController *controller,
                     const ::octf::proto::Void *request,
                     ::octf::proto::ChildrenList *response,
                     ::google::protobuf::Closure *done) override;

    void getInterfaces(::google::protobuf::RpcController *controller,
                       const ::octf::proto::Void *request,
                       ::octf::proto::InterfaceList *response,
                       ::google::protobuf::Closure *done) override;

private:
    INode *m_owner;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACEIDENTIFICATIONIMPL_H
