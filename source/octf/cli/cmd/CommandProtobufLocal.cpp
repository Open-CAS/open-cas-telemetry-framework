/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/cmd/CommandProtobufLocal.h>
#include <octf/cli/param/ParamEnum.h>
#include <octf/cli/param/ParamFlag.h>
#include <octf/cli/param/ParamNumber.h>
#include <octf/cli/param/ParamString.h>
#include <octf/communication/Call.h>
#include <octf/interface/InterfaceCliImpl.h>
#include <octf/node/INode.h>

namespace octf {

namespace protobuf = ::google::protobuf;
using namespace proto;
using namespace std;

CommandProtobufLocal::CommandProtobufLocal(
        const protobuf::MethodDescriptor *method,
        InterfaceShRef interface)
        : CommandProtobuf() {
    setLocal(true);
    setHidden(false);
    m_interface = interface;
    m_method = method;

    // Set keys and description
    setDesc(method->options().GetExtension(opts_command).cli_desc());
    setLongKey(method->options().GetExtension(opts_command).cli_long_key());
    setShortKey(method->options().GetExtension(opts_command).cli_short_key());

    // Validate keys
    if (!cliUtils::isLongKeyValid(getLongKey())) {
        throw Exception("Invalid long key for local protobuf command: " +
                        getLongKey());
    }
    if (!cliUtils::isShortKeyValid(getShortKey())) {
        throw Exception("Invalid short key for local protobuf command: " +
                        getShortKey());
    }

    // Add CLI parameters:
    // We utilize here autoCLI mechanism from base class to add CLI
    // parameter from method descriptor. To do so we need to convert
    // each FieldDescriptor of method's input message to CliParameter
    // as this is the type CommandProtobuf operates on
    const protobuf::Descriptor *inputMessage = method->input_type();
    for (int i = 0; i < inputMessage->field_count(); i++) {
        const auto inputField = inputMessage->field(i);
        proto::CliParameter param;
        InterfaceCliImpl::setParamDescription(&param, inputField, true);

        CommandProtobuf::createParameter(param);
    }
}

void CommandProtobufLocal::execute() {
    // Create mutable input message from prototype
    // Ownership passed to the caller, thus use smart pointer to handle it
    MessageShRef in_msg =
            MessageShRef(m_interface->GetRequestPrototype(m_method).New());

    // Parse parameters - set values read from cli to the protobuf message
    parseToProtobuf(in_msg.get(), m_method->input_type());

    // Create mutable output message from prototype
    // Ownership passed to the caller, thus use smart pointer to handle it
    MessageShRef out_msg =
            MessageShRef(m_interface->GetResponsePrototype(m_method).New());

    // Call method
    CallGeneric call(in_msg, out_msg);
    m_interface->CallMethod(m_method, &call, in_msg.get(), out_msg.get(),
                            &call);

    // Wait for call completion and print response
    handleCall(call, out_msg);
}

}  // namespace octf
