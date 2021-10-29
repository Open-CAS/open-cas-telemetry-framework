/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_METHOD_H
#define SOURCE_OCTF_COMMUNICATION_METHOD_H

#include <google/protobuf/stubs/callback.h>
#include <memory>
#include <string>
#include <octf/communication/RpcControllerImpl.h>
#include <octf/interface/InterfaceId.h>
#include <octf/node/INode.h>
#include <octf/node/NodeId.h>
#include <octf/proto/packets.pb.h>

namespace octf {

class ConnectionContext;

/**
 * @brief This class represents method which was received by service or plugin
 * and to be executed
 *
 * This class consist of all what is needed to execute method. E.g: input
 * parameter, output parameter, connection context on which send back response
 */
class Method : public google::protobuf::Closure {
public:
    /**
     * @param connCntx Connection Context on which send back response
     */
    Method(std::weak_ptr<ConnectionContext> connCntx);
    virtual ~Method() = default;

    /**
     * @brief Prepares method on the basis of received method packet
     *
     * @param request Method request packet
     *
     * @retval true Method prepared successfully
     * @retval false Method preparation failure because of an error in packet
     */
    bool prepareMethod(proto::PacketMethodRequest *request);

    /**
     * @brief Prepares input and output parameters for given method in given
     * interface
     *
     * @param interface Interface which contains the method for which
     * input and output parameters will be prepared
     *
     * @retval true parameters ready and no errors
     * @retval false parameters preparation failure, the potential error can be
     * parsing error of request payload into input parameter
     */
    bool prepareParameters(const InterfaceShRef &interface);

    /**
     * @brief Prepares response packet once method has been handled
     *
     * @param[out] packet Protocol buffer packet
     */
    void prepareResponsePacket(proto::Packet &packet);

    /**
     * @brief Gets id of interface on which method shall be executed
     *
     * @return Interface ID
     */
    const InterfaceId &getInterfaceId() const {
        return m_interfaceId;
    }

    /**
     * @brief Gets Method ID
     *
     * @return Method ID
     */
    int32_t getMethodId() const {
        return m_methodID;
    }

    /**
     * @brief Gets node path on which this method has to be executed
     *
     * @return Node path
     */
    const NodePath &getNodePath() const {
        return m_path;
    }

    /**
     * @brief Gets method sequence id
     *
     * @return method sequence id
     */
    uint32_t getSequenceId() const {
        return m_sequenceId;
    }

    /**
     * @brief Gets method input parameter
     *
     * @return Method input parameter
     */
    const google::protobuf::Message *getInput() const {
        return m_input.get();
    }

    /**
     * @brief Gets method output parameter
     *
     * @return Method output parameter
     */
    google::protobuf::Message *getOutput() {
        return m_output.get();
    }

    /**
     * @brief Gets method descriptor
     *
     * @return Protocol buffer method descriptor
     */
    const google::protobuf::MethodDescriptor *getMethodDescriptor(
            const InterfaceShRef &interface);

    /**
     * @brief Runs method completion
     *
     * This method override google::protobuf::Closure::Run() and is called
     * by interface when method response is ready and shall be sent to the
     * client
     */
    void Run() override;

    /**
     * @brief Fails method
     *
     * @param reason Failure reasons
     */
    inline void fail(const std::string &reason) {
        m_rpcControler.SetFailed(reason);
        this->Run();
    }

    /**
     * @brief Gets RPC controller
     *
     * @return RPC controller
     */
    RpcControllerImpl *getRpcController() {
        return &m_rpcControler;
    }

private:
    /**
     * Connection reference through which send back response
     */
    std::weak_ptr<ConnectionContext> m_connCntx;

    /**
     * Path of node on which execute this method
     */
    NodePath m_path;

    /**
     * Id of interface on which execute this method
     */
    InterfaceId m_interfaceId;

    /**
     * Method ID
     */
    int32_t m_methodID;

    /**
     * Method sequence id
     */
    uint32_t m_sequenceId;

    /**
     * method request (input parameter) payload
     */
    std::string m_requestPayload;

    /**
     * Method input type
     */
    std::unique_ptr<google::protobuf::Message> m_input;

    /**
     * Method output type
     */
    std::unique_ptr<google::protobuf::Message> m_output;

    /**
     * RPC controller
     *
     * It's needed for setting error on method
     */
    RpcControllerImpl m_rpcControler;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_METHOD_H
