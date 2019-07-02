/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUF_H
#define SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUF_H

#include <google/protobuf/descriptor.h>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/cli/internal/cmd/Command.h>
#include <octf/communication/Call.h>
#include <octf/interface/InterfaceId.h>
// TODO
#include <octf/proto/InterfaceCLI.pb.h>

namespace octf {
namespace cli {

/**
 * @brief Command related to Google Protobuf interfaces.
 */
class CommandProtobuf : public Command {
public:
    /**
     * @brief CommandProtobuf constructor using proto::CliCommand
     * @param cmdDesc Command description
     */
    CommandProtobuf(const proto::CliCommandDesc &cmdDesc);

    CommandProtobuf()
            : m_inDesc(nullptr)
            , m_outDesc(nullptr)
            , m_interfaceId()
            , m_methodId(){};

    virtual ~CommandProtobuf() = default;

    // EXTENDING METHODS
    /**
     * @return InterfaceId of interface which command
     * belongs to
     */
    virtual const InterfaceId &getInterfaceId() const {
        return m_interfaceId;
    };

    /**
     * @return Numeric ID of method in interface which is related to command
     */
    virtual int getMethodId() const {
        return m_methodId;
    };

    /**
     * @param id of interface which command
     * belongs to
     */
    virtual void setInterfaceId(const InterfaceId &id) {
        m_interfaceId = id;
    };

    /**
     * @param id Numeric ID of method in interface which is related to command
     */
    virtual void setMethodId(const int id) {
        m_methodId = id;
    };

    /**
     * @brief Gets input protocol buffer descriptor
     *
     * @return Input protocol buffer descriptor
     */
    const google::protobuf::Descriptor *getInputDesc();

    /**
     * @brief Gets output protocol buffer descriptor
     *
     * @return Output protocol buffer descriptor
     */
    const google::protobuf::Descriptor *getOutputDesc();

    /**
     * @brief Parses values in command's parameters and fills message based on
     * inputDescription
     *
     * @param[out] message Message to be filled with input (parameters' values)
     * @param inputDesc Description of input message
     */
    void parseToProtobuf(google::protobuf::Message *message,
                         const google::protobuf::Descriptor *inputDesc);

    /**
     * @brief Waits for call completion and prints response
     *
     * @param call Call to wait for
     * @param outMessage Message to be printed
     */
    virtual void handleCall(CallGeneric &call, MessageShRef outMessage);

protected:
    /**
     * @brief Sets descriptor of input protobuf message for command
     *
     * @param desc Input message descriptor
     */
    void setInputDesc(const google::protobuf::Descriptor *desc);

    /**
     * @brief Sets descriptor of output protobuf message for command
     *
     * @param desc Output message descriptor
     */
    void setOutputDesc(const google::protobuf::Descriptor *desc);

    /**
     * @brief Creates parameters and adds them to this command based on method
     * input descriptor.
     */
    void createParameters();

private:
    //    /**
    //     * @brief Returns FieldDescriptor::Type from CliParameter_Type
    //     * @param type Type to be converted
    //     * @return value as FieldDescriptor::Type
    //     */
    //    virtual google::protobuf::FieldDescriptor::Type
    //    getFieldDescriptorType(
    //            proto::CliParameter_Type type);
    //
    //    void addParamDescProto(const std::string &name,
    //                           google::protobuf::DescriptorProto *desc,
    //                           const proto::CliParameter &param);
    //
    //    void addInputParamDescProto(google::protobuf::DescriptorProto *desc,
    //                                const proto::CliParameter &param);
    //
    //    void addOutputParamDescProto(google::protobuf::DescriptorProto *desc,
    //                                 const proto::CliParameter &param);
    //
    //    void addEnumParamDescProto(const std::string &name,
    //                               const proto::OptsParam &paramOpts);
    //
    //    void addMessageParamDescProto(const std::string &messageName,
    //                                  google::protobuf::DescriptorProto *desc,
    //                                  const proto::CliParameter &param);

private:
    /**
     * Not used, empty implementation.
     */
    void execute() override{};

private:
    /**
     * Protocol buffer file descriptor which is used to create runtime file
     * descriptor
     */
    google::protobuf::FileDescriptorProto m_fd;

    /**
     * Pool with descriptors of input and output messages
     */
    google::protobuf::DescriptorPool m_descPool;

    /**
     * Input protocol buffer descriptor
     */
    const google::protobuf::Descriptor *m_inDesc;

    /**
     * Output protocol buffer descriptor
     */
    const google::protobuf::Descriptor *m_outDesc;

    /**
     * ID of interface on which this command will be called
     */
    InterfaceId m_interfaceId;

    /**
     * ID of corresponding method within interface
     */
    int m_methodId;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CMD_COMMANDPROTOBUF_H
