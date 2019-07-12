/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/descriptor.h>
#include <exception>
#include <octf/cli/internal/OptionsValidation.h>
#include <octf/interface/InterfaceCliImpl.h>
#include <octf/utils/Exception.h>
#include <octf/utils/ProtoConverter.h>

namespace octf {

void InterfaceCliImpl::getCliCommandSetDescription(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::CliCommandSetDesc *response,
        ::google::protobuf::Closure *done) {
    // request is Void and not used
    (void) request;

    InterfacesIdList interfacesId;
    google::protobuf::FileDescriptorProto protoFileDesc;

    try {
        bool result = m_owner->getInterfacesIdList(interfacesId);
        if (!result) {
            throw Exception("Interface error.");
        }
        for (auto id : interfacesId) {
            auto interface = m_owner->getInterface(id);
            if (!interface) {
                // TODO (jstencel) Log an error?

                // Skip this interface ID, actual interface object doesn't exist
                continue;
            }
            const google::protobuf::ServiceDescriptor *interfaceDesc =
                    interface->GetDescriptor();

            if (!isValidInterface(interfaceDesc)) {
                // Don't include this interface in command set
                continue;
            }

            int methodCount = interfaceDesc->method_count();

            for (int methodIndex = 0; methodIndex < methodCount;
                 methodIndex++) {
                // Get description of method
                auto methodDesc = interfaceDesc->method(methodIndex);
                if (!isValidMethod(methodDesc)) {
                    // Don't include this method in command set
                    continue;
                }

                // Add Command to CommandSet
                proto::CliCommandDesc *cmd = response->add_command();

                // Set description for command
                if (!setCommandDescription(cmd, id, methodIndex, methodDesc)) {
                    // Invalid description of specific method, skip it
                    response->mutable_command()->RemoveLast();
                    continue;
                }
            }
            const octf::proto::CliCommandSetDesc &cmdSet = *response;
            // Check validness of whole created command set
            if (!cli::utils::isCommandSetValid(cmdSet, true)) {
                throw Exception("Not valid command set.");
            }
        }
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    // Response (command set) ready, send it back
    // It's done both for sending correct response and informing
    // about some failure during execution of command
    done->Run();
}

void InterfaceCliImpl::getCliCommandDescription(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::CliCommandId *request,
        ::octf::proto::CliCommandDesc *response,
        ::google::protobuf::Closure *done) {
    InterfacesIdList interfacesId;
    bool isCmdFound = false;
    try {
        bool result = m_owner->getInterfacesIdList(interfacesId);
        if (!result) {
            throw Exception("Interface error.");
        }
        for (auto id : interfacesId) {
            auto interface = m_owner->getInterface(id);
            if (!interface) {
                // TODO (jstencel) Log an error?

                // Skip this interface ID, actual interface object doesn't exist
                continue;
            }
            const google::protobuf::ServiceDescriptor *interfaceDesc =
                    interface->GetDescriptor();
            if (!isValidInterface(interfaceDesc)) {
                continue;
            }

            int methodCount = interfaceDesc->method_count();
            // TODO (jstencel) Enable not standard enumeration
            // e.g. three methods with indexes: 4, 9, 13
            for (int methodIndex = 0; methodIndex < methodCount;
                 methodIndex++) {
                // Get description of method
                auto methodDesc = interfaceDesc->method(methodIndex);
                if (!methodDesc) {
                    continue;
                }
                const auto &methodOps =
                        methodDesc->options().GetExtension(proto::opts_command);
                if (methodOps.cli_long_key() == request->commandkey() ||
                    methodOps.cli_short_key() == request->commandkey()) {
                    if (!isValidMethod(methodDesc)) {
                        // Invalid method description
                        break;
                    }
                    isCmdFound = true;
                    bool success = setCommandDescription(
                            response, id, methodIndex, methodDesc);
                    if (!success) {
                        throw Exception("Wrong command CLI description.");
                    }
                    break;
                }
            }
        }
        if (!isCmdFound) {
            controller->SetFailed("Unknown command");
        }
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    // Response (command) ready, send it back
    // It's done both for sending correct response and informing
    // about some failure during execution of command
    done->Run();
}

bool InterfaceCliImpl::setCommandDescription(
        proto::CliCommandDesc *cmd,
        const InterfaceId &id,
        int methodIndex,
        const google::protobuf::MethodDescriptor *methodDesc) {
    // Set all required parameters to method (command) description

    // TODO (jstencel) Add validation checks for NodeId and all other parameters
    // before setting them in description
    protoconverter::convertNodePath(cmd->mutable_path(),
                                    m_owner->getNodePath());

    // InterfaceId
    protoconverter::convertInterfaceId(cmd->mutable_interface(), id);

    // MethodId
    cmd->set_methodid(methodIndex);

    // SaOptCommand
    if (methodDesc->options().HasExtension(proto::opts_command)) {
        const auto &opsCmd =
                methodDesc->options().GetExtension(proto::opts_command);
        cmd->mutable_cmdops()->CopyFrom(opsCmd);
    }

    // Input/Output types description
    // We generate a Set of FileDescriptors which contain definitions of input
    // and output messages and all enums or messages used within. We also set
    // what message is to be used as input and which as output.
    google::protobuf::FileDescriptorSet fdSet;
    protoconverter::FileDescriptorSetCreator fdSetCreator(fdSet);
    fdSetCreator.addMessageDesc(methodDesc->input_type());
    fdSetCreator.addMessageDesc(methodDesc->output_type());

    cmd->mutable_inputoutputdescription()->CopyFrom(fdSet);

    cmd->set_inputmessagetypename(methodDesc->input_type()->full_name());
    cmd->set_outputmessagetypename(methodDesc->output_type()->full_name());

    return true;
}

bool InterfaceCliImpl::isValidInterface(
        const google::protobuf::ServiceDescriptor *interfaceDesc) {
    if (!interfaceDesc) {
        return false;
    }
    const auto &opsInterface =
            interfaceDesc->options().GetExtension(proto::opts_interface);
    if (!opsInterface.cli()) {
        return false;
    }

    // All checks passed successfully, interface is valid.
    return true;
}

bool InterfaceCliImpl::isValidMethod(
        const google::protobuf::MethodDescriptor *methodDesc) {
    if (!methodDesc) {
        return false;
    }
    if (!methodDesc->options().HasExtension(proto::opts_command)) {
        return false;
    }
    const auto &opsMethod =
            methodDesc->options().GetExtension(proto::opts_command);
    if (!opsMethod.cli()) {
        return false;
    }

    if (!cli::utils::isShortKeyValid(opsMethod.cli_short_key())) {
        return false;
    }
    if (!cli::utils::isLongKeyValid(opsMethod.cli_long_key())) {
        return false;
    }
    if (!cli::utils::isDescValid(opsMethod.cli_desc())) {
        return false;
    }

    // All checks passed successfully, method is valid.
    return true;
}

}  // namespace octf
