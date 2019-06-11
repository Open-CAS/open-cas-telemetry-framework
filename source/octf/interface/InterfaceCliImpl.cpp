/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceCliImpl.h>

#include <google/protobuf/descriptor.h>
#include <exception>
#include <octf/cli/internal/OptionsValidation.h>
#include <octf/utils/Exception.h>
#include <octf/utils/ProtoConverter.h>

typedef google::protobuf::FieldDescriptor::CppType CppType;

namespace octf {
void InterfaceCliImpl::getCliCommandSetDescription(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::CliCommandSet *response,
        ::google::protobuf::Closure *done) {
    // request is Void and not used
    (void) request;

    InterfacesIdList interfacesId;

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
                proto::CliCommand *cmd = response->add_command();
                // Set description for command
                if (!setCommandDescription(cmd, id, methodIndex, methodDesc)) {
                    // Invalid description of specific method, skip it
                    response->mutable_command()->RemoveLast();
                    continue;
                }
            }
            const octf::proto::CliCommandSet &cmdSet = *response;
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
        ::octf::proto::CliCommand *response,
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
        proto::CliCommand *cmd,
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

    // CliParameter-s - input
    auto mutableInput = cmd->mutable_inputparameter();
    auto inputDesc = methodDesc->input_type();

    if (!setParamsDescription(mutableInput, inputDesc)) {
        return false;
    }

    // CliParameter-s - output
    auto mutableOutput = cmd->mutable_outputparameter();
    auto outputDesc = methodDesc->output_type();
    if (!setParamsDescription(mutableOutput, outputDesc, false)) {
        return false;
    }

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

void InterfaceCliImpl::setDefaultValues(
        proto::CliParameter *param,
        const google::protobuf::FieldDescriptor *fieldDesc,
        proto::CliParameter_Type type) {
    // Set defaults for missing values in enum
    if (type == proto::CliParameter_Type::CliParameter_Type_ENUM) {
        const auto &opsParamEnum =
                fieldDesc->enum_type()->options().GetExtension(
                        proto::opts_enum_param);

        proto::OptsParam *mutableParamOps = param->mutable_paramops();

        if (mutableParamOps->cli_desc().empty()) {
            param->mutable_paramops()->set_cli_desc(opsParamEnum.cli_desc());
        }

        if (mutableParamOps->cli_long_key().empty()) {
            param->mutable_paramops()->set_cli_long_key(
                    opsParamEnum.cli_long_key());
        }

        if (mutableParamOps->cli_short_key().empty()) {
            param->mutable_paramops()->set_cli_short_key(
                    opsParamEnum.cli_short_key());
        }

        // TODO (jstencel) Consider how to recognize unset bool from
        // the one explicitly set to zero
        if (!mutableParamOps->cli_required()) {
            param->mutable_paramops()->set_cli_required(
                    opsParamEnum.cli_required());
        }
    }
}

bool InterfaceCliImpl::setParamsDescription(
        google::protobuf::RepeatedPtrField<proto::CliParameter>
                *mutableCliParam,
        const google::protobuf::Descriptor *desc,
        bool isInput) {
    for (int paramIndex = 0; paramIndex < desc->field_count(); paramIndex++) {
        auto fieldDesc = desc->field(paramIndex);

        if (!fieldDesc) {
            throw Exception("Incorrect interface.");
        }

        // Add parameter to command's input/output description
        proto::CliParameter *param = mutableCliParam->Add();

        // Map CliParameter to descriptor's field
        if (!setParamDescription(param, fieldDesc, isInput)) {
            return false;
        }
    }
    return true;
}

bool InterfaceCliImpl::setParamDescription(
        proto::CliParameter *param,
        const google::protobuf::FieldDescriptor *fieldDesc,
        bool isInput) {
    // Set type
    auto cppType = fieldDesc->cpp_type();
    proto::CliParameter_Type type = getTypeFromCppType(cppType);

    param->set_type(type);
    param->set_fieldid(fieldDesc->number());

    // SaOptParam
    const auto &opsParam = fieldDesc->options().GetExtension(proto::opts_param);

    // Set values from interface definition
    param->mutable_paramops()->CopyFrom(opsParam);

    if (type == proto::CliParameter_Type::CliParameter_Type_ENUM) {
        auto enumDesc = fieldDesc->enum_type();
        setEnumOps(enumDesc, param);

    } else if (type == proto::CliParameter_Type::CliParameter_Type_MESSAGE) {
        // Field is a message, create message type parameter
        const google::protobuf::Descriptor *messageFieldDesc =
                fieldDesc->message_type();

        // Fill nested CliParameter with fields from descriptor
        for (int fieldIndex = 0; fieldIndex < messageFieldDesc->field_count();
             fieldIndex++) {
            auto nestedParam = param->add_nestedparam();
            setParamDescription(nestedParam,
                                messageFieldDesc->field(fieldIndex), false);
        }
    }

    // Add label for repeated fields
    param->mutable_paramops()->set_is_repeated(fieldDesc->is_repeated());

    // Complete missing values with default ones
    setDefaultValues(param, fieldDesc, type);

    if (isInput) {
        // Check correctness of input parameter's options
        if (!cli::utils::isDescValid(param->mutable_paramops()->cli_desc())) {
            return false;
        }
        if (!cli::utils::isShortKeyValid(
                    param->mutable_paramops()->cli_short_key())) {
            return false;
        }
        if (!cli::utils::isLongKeyValid(
                    param->mutable_paramops()->cli_long_key())) {
            return false;
        }
    } else {
        // Building output description...
        //
        // For message:
        // message HelloWorldResponse {
        //      string helloResponse = 1;
        //      someEnum enumResponse = 2;
        //      someMessage messageResponse = 3;
        // }
        //
        // JSON output shall be:
        // {
        //      "helloResponse": "Test response",
        //      "enumResponse": "foo",
        //      "messageReponse":
        //      {
        //          "fieldName": "Test response"
        //      }
        //  }
        //
        // So we have to copy fields' names. We store them in parameter's
        // paramOps. To output a nested message we also need to send message
        // definition. To do so without duplication of definitions, if field
        // is a message, we keep the message's name in paramOps.

        // Set name of the message
        if (fieldDesc->type() ==
            google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
            param->mutable_paramops()->mutable_cli_msg()->set_message_name(
                    fieldDesc->message_type()->name());
        }

        // Set field name
        param->mutable_paramops()->set_field_name(fieldDesc->name());
    }

    return true;
}

void InterfaceCliImpl::setEnumOps(
        const google::protobuf::EnumDescriptor *enumDesc,
        proto::CliParameter *param) {
    if (enumDesc == nullptr) {
        throw Exception("Incorrect CLI description.");
    }

    auto *enumOps = param->mutable_paramops()->mutable_cli_enum();
    enumOps->set_enum_name(enumDesc->name());

    for (int enumValueIndex = 0; enumValueIndex < enumDesc->value_count();
         enumValueIndex++) {
        auto enumValDesc = enumDesc->value(enumValueIndex);

        if (!enumValDesc) {
            throw Exception("Incorrect interface.");
        }

        const auto &opsEnumVal =
                enumValDesc->options().GetExtension(proto::opts_enumval);

        proto::OptsEnumValue *enumV = enumOps->add_enum_value();

        enumV->CopyFrom(opsEnumVal);
        enumV->set_value(enumValDesc->number());
    }
}

proto::CliParameter_Type InterfaceCliImpl::getTypeFromCppType(
        google::protobuf::FieldDescriptor::CppType cppType) {
    switch (cppType) {
    case CppType::CPPTYPE_INT32:
        return proto::CliParameter_Type::CliParameter_Type_INT32;

    case CppType::CPPTYPE_INT64:
        return proto::CliParameter_Type::CliParameter_Type_INT64;

    case CppType::CPPTYPE_UINT32:
        return proto::CliParameter_Type::CliParameter_Type_UINT32;

    case CppType::CPPTYPE_ENUM:
        return proto::CliParameter_Type::CliParameter_Type_ENUM;

    case CppType::CPPTYPE_STRING:
        return proto::CliParameter_Type::CliParameter_Type_STRING;

    case CppType::CPPTYPE_BOOL:
        return proto::CliParameter_Type::CliParameter_Type_FLAG;

    case CppType::CPPTYPE_MESSAGE:
        return proto::CliParameter_Type::CliParameter_Type_MESSAGE;

    case CppType::CPPTYPE_UINT64:
    case CppType::CPPTYPE_FLOAT:
    case CppType::CPPTYPE_DOUBLE:

    default:
        // Not explicitly supported type
        // TODO (jstencel) Consider if it should be passed in some form
        // or if error should occur in such case
        return proto::CliParameter_Type::CliParameter_Type_UNKNOWN;
    }
}
}  // namespace octf
