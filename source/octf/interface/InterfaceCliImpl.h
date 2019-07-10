/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACECLIIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACECLIIMPL_H

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <octf/node/INode.h>
#include <octf/proto/InterfaceCLI.pb.h>

namespace octf {

class InterfaceCliImpl : public proto::InterfaceCli {
public:
    InterfaceCliImpl(INode *owner)
            : m_owner(owner) {}

    virtual ~InterfaceCliImpl() = default;

    void getCliCommandSetDescription(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::Void *request,
            ::octf::proto::CliCommandSetDesc *response,
            ::google::protobuf::Closure *done) override;

    void getCliCommandDescription(::google::protobuf::RpcController *controller,
                                  const ::octf::proto::CliCommandId *request,
                                  ::octf::proto::CliCommandDesc *response,
                                  ::google::protobuf::Closure *done) override;

private:
    /**
     * @param[out] cmd
     *
     * @return True if description of command finished successfully;
     * False otherwise.
     */
    bool setCommandDescription(
            proto::CliCommandDesc *cmd,
            const InterfaceId &id,
            int methodIndex,
            const google::protobuf::MethodDescriptor *methodDesc);

    bool isValidInterface(
            const google::protobuf::ServiceDescriptor *interfaceDesc);

    bool isValidMethod(const google::protobuf::MethodDescriptor *methodDesc);

    INode *m_owner;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACECLIIMPL_H
