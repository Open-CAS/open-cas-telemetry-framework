/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/OptionsValidation.h>
#include <octf/cli/internal/cmd/CommandProtobufLocal.h>
#include <octf/cli/internal/param/ParamEnum.h>
#include <octf/cli/internal/param/ParamFlag.h>
#include <octf/cli/internal/param/ParamNumber.h>
#include <octf/cli/internal/param/ParamString.h>
#include <octf/communication/Call.h>
#include <octf/interface/InterfaceCliImpl.h>
#include <octf/node/INode.h>

namespace octf {
namespace cli {

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

    // Save input and output types description to later create appropriate
    // CLI parameters
    setInputDesc(method->input_type());
    setOutputDesc(method->output_type());

    // Set keys and description
    setDesc(method->options().GetExtension(opts_command).cli_desc());
    setLongKey(method->options().GetExtension(opts_command).cli_long_key());
    setShortKey(method->options().GetExtension(opts_command).cli_short_key());

    // Validate keys
    if (!utils::isLongKeyValid(getLongKey())) {
        throw Exception("Invalid long key for local protobuf command: " +
                        getLongKey());
    }
    if (!utils::isShortKeyValid(getShortKey())) {
        throw Exception("Invalid short key for local protobuf command: " +
                        getShortKey());
    }

    CommandProtobuf::createParameters();
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

}  // namespace cli
}  // namespace octf
