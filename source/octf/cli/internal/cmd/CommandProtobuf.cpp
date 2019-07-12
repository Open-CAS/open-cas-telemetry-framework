/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <memory>
#include <vector>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/cli/internal/OptionsValidation.h>
#include <octf/cli/internal/cmd/CommandProtobuf.h>
#include <octf/cli/internal/param/ParamEnum.h>
#include <octf/cli/internal/param/ParamFlag.h>
#include <octf/cli/internal/param/ParamNumber.h>
#include <octf/cli/internal/param/ParamString.h>
#include <octf/cli/internal/param/Parameter.h>
#include <octf/communication/Call.h>
#include <octf/proto/InterfaceCLI.pb.h>
#include <octf/utils/Exception.h>

using google::protobuf::FieldDescriptor;

using namespace std;

namespace octf {
namespace cli {

CommandProtobuf::CommandProtobuf(const proto::CliCommandDesc &cmdDesc)
        : Command()
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
    if (!utils::isShortKeyValid(cmdDesc.cmdops().cli_short_key())) {
        throw Exception("Invalid short key for command.");
    }
    setShortKey(cmdDesc.cmdops().cli_short_key());

    if (!utils::isLongKeyValid(cmdDesc.cmdops().cli_long_key())) {
        throw Exception("Invalid long key for command.");
    }
    setLongKey(cmdDesc.cmdops().cli_long_key());

    // Set interface and method ID
    m_interfaceId = cmdDesc.interface();
    m_methodId = cmdDesc.methodid();

    // Build file descriptor
    for (int i = 0; i < cmdDesc.inputoutputdescription().file_size(); i++) {
        const google::protobuf::FileDescriptorProto fd =
                cmdDesc.inputoutputdescription().file(i);

        auto createdFd = m_descPool.BuildFile(fd);
        if (!createdFd) {
            // Cannot prepare file descriptor which is required
            // for creation input and output types
            throw InvalidParameterException(
                    "Cannot build input and output types descriptions.");
        }
    }

    m_inDesc = m_descPool.FindMessageTypeByName(cmdDesc.inputmessagetypename());
    if (!m_inDesc) {
        throw InvalidParameterException(
                "Cannot find message: " + cmdDesc.inputmessagetypename() +
                " to build input message description");
    }

    m_outDesc =
            m_descPool.FindMessageTypeByName(cmdDesc.outputmessagetypename());
    if (!m_outDesc) {
        throw InvalidParameterException(
                "Cannot find message: " + cmdDesc.outputmessagetypename() +
                " to build output message description");
    }

    // Add parameters for command based on received command description
    createParameters();
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

const google::protobuf::Descriptor *CommandProtobuf::getInputDesc() const {
    return m_inDesc;
}

const google::protobuf::Descriptor *CommandProtobuf::getOutputDesc() const {
    return m_outDesc;
}

void CommandProtobuf::setInputDesc(const google::protobuf::Descriptor *desc) {
    m_inDesc = desc;
}

void CommandProtobuf::setOutputDesc(const google::protobuf::Descriptor *desc) {
    m_outDesc = desc;
}

void CommandProtobuf::createParameters() {
    for (int fieldId = 0; fieldId < m_inDesc->field_count(); fieldId++) {
        google::protobuf::FieldDescriptor::Type type =
                m_inDesc->field(fieldId)->type();

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

        param->setOptions(m_inDesc->field(fieldId));
        param->setIndex(m_inDesc->field(fieldId)->number());
        param->setMultipleValue(m_inDesc->field(fieldId)->is_repeated());
        if (m_inDesc->field(fieldId)->is_repeated() &&
            !param->isMultipleValue()) {
            throw InvalidParameterException(
                    "CLI parameter doesn't support multiple"
                    " values requested in .proto definition via 'repeated'.");
        }

        addParam(param);
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
        utils::printOutputMessage(outMessage);
    }
}

}  // namespace cli
}  // namespace octf
