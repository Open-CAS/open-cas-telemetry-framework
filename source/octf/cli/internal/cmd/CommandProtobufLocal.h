/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUFLOCAL_H
#define SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUFLOCAL_H

#include <string>
#include <octf/cli/internal/cmd/Command.h>
#include <octf/cli/internal/cmd/CommandProtobuf.h>
#include <octf/node/INode.h>

namespace octf {
namespace cli {

/**
 * @brief Command for executing interfaces defined in .proto files locally
 */
class CommandProtobufLocal : public CommandProtobuf {
public:
    /**
     * @brief Create local command with given interface and method
     *
     * @param method Pointer to method
     * @param interface Pointer to interface
     */
    CommandProtobufLocal(const ::google::protobuf::MethodDescriptor *method,
                         InterfaceShRef interface);

    virtual ~CommandProtobufLocal() = default;

    /**
     * @brief Execute method associated with this command
     */
    void execute() override;

private:
    InterfaceShRef m_interface;
    const ::google::protobuf::MethodDescriptor *m_method;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUFLOCAL_H
