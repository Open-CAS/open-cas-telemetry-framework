/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/io/tokenizer.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <memory>
#include <vector>
#include <octf/cli/CLIUtils.h>
#include <octf/cli/cmd/CommandProtobuf.h>
#include <octf/cli/param/ParamEnum.h>
#include <octf/cli/param/ParamFlag.h>
#include <octf/cli/param/ParamNumber.h>
#include <octf/cli/param/ParamString.h>
#include <octf/cli/param/Parameter.h>
#include <octf/communication/Call.h>
#include <octf/proto/InterfaceCLI.pb.h>
#include <octf/utils/Exception.h>

using google::protobuf::FieldDescriptor;

using namespace std;

namespace octf {

CommandProtobuf::CommandProtobuf(const proto::CliCommand &cmdDesc)
        : Command()
        , m_fd()
        , m_descPool()
        , m_inDesc(nullptr)
        , m_outDesc(nullptr)
        , m_interfaceId()
        , m_methodId(-1) {
    // Create command for AutoCLI based on received command description
    setLocal(false);
    setHidden(false);
    setDesc(cmdDesc.cmdops().cli_desc());

    // Validate short and long key for command
    if (!cliUtils::isShortKeyValid(cmdDesc.cmdops().cli_short_key())) {
        throw Exception("Invalid short key for command.");
    }
    setShortKey(cmdDesc.cmdops().cli_short_key());

    if (!cliUtils::isLongKeyValid(cmdDesc.cmdops().cli_long_key())) {
        throw Exception("Invalid long key for command.");
    }
    setLongKey(cmdDesc.cmdops().cli_long_key());

    // Set interface and method ID
    m_interfaceId = cmdDesc.interface();
    m_methodId = cmdDesc.methodid();

    // Set name of file descriptor
    m_fd.set_name("cli");

    // Instantiate input type and set name
    auto inputMsgDescProto = m_fd.add_message_type();
    inputMsgDescProto->set_name("input");

    // Instantiate output type and set name
    auto outputMsgDescProto = m_fd.add_message_type();
    outputMsgDescProto->set_name("output");

    // Add parameters for command based on received command description
    int count = cmdDesc.inputparameter_size();
    for (int i = 0; i < count; i++) {
        const proto::CliParameter &paramDef = cmdDesc.inputparameter(i);

        // Create and add a parameter based on definition in CliParameter
        createParameter(paramDef);

        // Add this parameter into input protocol buffer descriptor
        addInputParamDescProto(inputMsgDescProto, paramDef);
    }

    // Build output protocol buffer parameter
    count = cmdDesc.outputparameter_size();
    for (int i = 0; i < count; i++) {
        addOutputParamDescProto(outputMsgDescProto, cmdDesc.outputparameter(i));
    }

    // Build file descriptor
    auto fd = m_descPool.BuildFile(m_fd);
    if (!fd) {
        // cannot prepare file descriptor which is required
        // for creation input and output types
        throw InvalidParameterException(
                "Cannot build input and output parameters descriptions.");
    }

    m_inDesc = m_descPool.FindMessageTypeByName("input");
    if (!m_inDesc) {
        throw InvalidParameterException(
                "Cannot build input parameter description.");
    }

    m_outDesc = m_descPool.FindMessageTypeByName("output");
    if (!m_outDesc) {
        throw InvalidParameterException(
                "Cannot build output parameter description.");
    }
}

void CommandProtobuf::parseToProtobuf(
        google::protobuf::Message *message,
        const google::protobuf::Descriptor *inputDesc) {
    // Fields are expected to have continuous enumeration.
    for (int i = 0; i < inputDesc->field_count(); ++i) {
        const FieldDescriptor *field = inputDesc->field(i);
        auto param = getParamByIndex(field->number());

        param->parseToProtobuf(message, field);
    }
}

void CommandProtobuf::addParamDescProto(const std::string &name,
                                        google::protobuf::DescriptorProto *desc,
                                        const proto::CliParameter &param) {
    // Allocate new field and set basic field features
    auto field = desc->add_field();
    field->set_name(name);
    field->set_number(param.fieldid());

    if (param.paramops().is_repeated()) {
        field->set_label(
                google::protobuf::FieldDescriptorProto_Label_LABEL_REPEATED);
    }

    switch (param.type()) {
    case proto::CliParameter_Type_INT32:
        field->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_INT32);
        break;
    case proto::CliParameter_Type_INT64:
        field->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_INT64);
        break;
    case proto::CliParameter_Type_UINT32:
        field->set_type(
                google::protobuf::FieldDescriptorProto_Type_TYPE_UINT32);
        break;
    case proto::CliParameter_Type_UINT64:
        field->set_type(
                google::protobuf::FieldDescriptorProto_Type_TYPE_UINT64);
        break;
    case proto::CliParameter_Type_STRING:
        field->set_type(
                google::protobuf::FieldDescriptorProto_Type_TYPE_STRING);
        break;
    case proto::CliParameter_Type_FLAG:
        field->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_BOOL);
        break;
    case proto::CliParameter_Type_ENUM: {
        field->set_type(google::protobuf::FieldDescriptorProto_Type_TYPE_ENUM);

        if (!param.paramops().has_cli_enum()) {
            throw InvalidParameterException("Missing enumerator CLI options.");
        }

        string enumTypeName = param.paramops().cli_enum().enum_name();

        field->set_type_name(enumTypeName);
        addEnumParamDescProto(enumTypeName, param.paramops());

        break;
    }

    case proto::CliParameter_Type_MESSAGE: {
        field->set_type(
                google::protobuf::FieldDescriptorProto_Type_TYPE_MESSAGE);

        if (!param.paramops().has_cli_msg()) {
            throw InvalidParameterException(
                    "Missing message's field CLI options.");
        }

        const std::string &messageName =
                param.paramops().cli_msg().message_name();
        field->set_type_name(messageName);
        addMessageParamDescProto(messageName, desc, param);
        break;
    }

    default:
        throw InvalidParameterException("Not supported field type.");
        break;
    }
}

void CommandProtobuf::addMessageParamDescProto(
        const std::string &messageName,
        google::protobuf::DescriptorProto *desc,
        const proto::CliParameter &param) {
    // Check if type is already defined
    bool defined = false;
    for (int i = 0; i < desc->nested_type_size(); i++) {
        if (desc->nested_type(i).name() == messageName) {
            defined = true;
            break;
        }
    }

    // Add message definition
    if (!defined) {
        auto messageType = desc->add_nested_type();
        messageType->set_name(messageName);

        // Copy nested parameters of this parameter as fields of message
        int nestedCount = param.nestedparam_size();
        for (int i = 0; i < nestedCount; i++) {
            const auto &nestedParam = param.nestedparam(i);

            // Recursively add nested message's fields
            addParamDescProto(nestedParam.paramops().field_name(), messageType,
                              nestedParam);
        }
    }
}

const google::protobuf::Descriptor *CommandProtobuf::getInputDesc() {
    return m_inDesc;
}

const google::protobuf::Descriptor *CommandProtobuf::getOutputDesc() {
    return m_outDesc;
}

void CommandProtobuf::addInputParamDescProto(
        google::protobuf::DescriptorProto *desc,
        const proto::CliParameter &param) {
    // Parameters in input message (fields in message for input type are
    // identified by long key)

    const auto &key = param.paramops().cli_long_key();
    if (key.empty()) {
        throw InvalidParameterException("Empty short key.");
    }

    addParamDescProto(key, desc, param);
}

void CommandProtobuf::addOutputParamDescProto(
        google::protobuf::DescriptorProto *desc,
        const proto::CliParameter &param) {
    // In output parameter paramOps hold field name
    addParamDescProto(param.paramops().field_name(), desc, param);
}

void CommandProtobuf::addEnumParamDescProto(const std::string &name,
                                            const proto::OptsParam &paramOpts) {
    // Check if enum type already added
    for (int i = 0; i < m_fd.enum_type_size(); i++) {
        const auto &enumDesc = m_fd.enum_type(i);
        if (enumDesc.name() == name) {
            // Enumerator already added, nothing to do
            return;
        }
    }

    // Allocate new enumerator in file descriptor
    auto enumDesc = m_fd.add_enum_type();
    enumDesc->set_name(name);

    // Add enumerator values
    for (int i = 0; i < paramOpts.cli_enum().enum_value_size(); i++) {
        const auto &eValueOpt = paramOpts.cli_enum().enum_value(i);

        const auto &cli_switch = eValueOpt.cli_switch();
        if (cli_switch.empty()) {
            throw InvalidParameterException(
                    "Empty CLI switch for enumerator value.");
        }

        // Allocate enumerator value and set its basic features
        auto eValueDesc = enumDesc->add_value();

        eValueDesc->set_name(cli_switch);
        eValueDesc->set_number(eValueOpt.value());
    }
}

void CommandProtobuf::createParameter(const proto::CliParameter &paramDef) {
    google::protobuf::FieldDescriptor::Type type =
            getFieldDescriptorType(paramDef.type());

    shared_ptr<IParameter> param;
    switch (type) {
    case google::protobuf::FieldDescriptor::Type::TYPE_UINT32:
    case google::protobuf::FieldDescriptor::Type::TYPE_INT64:
    case google::protobuf::FieldDescriptor::Type::TYPE_INT32: {
        // Create number parameter
        param = make_shared<ParamNumber>();
        break;
    }

    case google::protobuf::FieldDescriptor::Type::TYPE_STRING: {
        // Create string parameter
        param = make_shared<ParamString>();
        break;
    }

    case google::protobuf::FieldDescriptor::Type::TYPE_BOOL: {
        // Create flag parameter
        param = make_shared<ParamFlag>();
        break;
    }

    case google::protobuf::FieldDescriptor::Type::TYPE_ENUM: {
        // Create enum parameter
        param = make_shared<ParamEnum>();
        break;
    }

    // Uint64 not supported
    case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
    default: {
        throw InvalidParameterException("Parameter type not supported.");
    }
    }

    param->setOptions(paramDef);
    param->setIndex(paramDef.fieldid());
    param->setMultipleValue(paramDef.ismultiplevalue());
    if (paramDef.ismultiplevalue() && !param->isMultipleValue()) {
        throw InvalidParameterException(
                "CLI parameter doesn't support multiple"
                " values requested in .proto definition.");
    }
    addParam(param);
}

google::protobuf::FieldDescriptor::Type CommandProtobuf::getFieldDescriptorType(
        proto::CliParameter_Type type) {
    using namespace google::protobuf;
    switch (type) {
    case proto::CliParameter_Type::CliParameter_Type_UINT32:
        return FieldDescriptor::Type::TYPE_UINT32;

    case proto::CliParameter_Type::CliParameter_Type_INT64:
        return FieldDescriptor::Type::TYPE_INT64;

    case proto::CliParameter_Type::CliParameter_Type_INT32:
        return FieldDescriptor::Type::TYPE_INT32;

    case proto::CliParameter_Type::CliParameter_Type_STRING:
        return FieldDescriptor::Type::TYPE_STRING;

    case proto::CliParameter_Type::CliParameter_Type_FLAG:
        return FieldDescriptor::Type::TYPE_BOOL;

    case proto::CliParameter_Type::CliParameter_Type_ENUM:
        return FieldDescriptor::Type::TYPE_ENUM;

    case proto::CliParameter_Type::CliParameter_Type_UINT64:
        return FieldDescriptor::Type::TYPE_UINT64;

    case proto::CliParameter_Type::CliParameter_Type_MESSAGE:
        return FieldDescriptor::Type::TYPE_MESSAGE;

    default:
        return FieldDescriptor::Type::MAX_TYPE;
    }
}

void CommandProtobuf::handleCall(CallGeneric &call,
                                 const MessageShRef outMessage) {
    // Wait for method's execution completion
    call.waitFor();

    if (call.Failed()) {
        // Method call failed
        throw Exception(call.ErrorText());
    } else {
        CLIUtils::printOutputMessage(outMessage);
    }
}

}  // namespace octf
