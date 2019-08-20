/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_INTERNAL_METHODCLIENT_H
#define SOURCE_OCTF_COMMUNICATION_INTERNAL_METHODCLIENT_H
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <stdint.h>
#include <memory>
#include <octf/interface/InterfaceId.h>
#include <octf/node/INode.h>
#include <octf/proto/defs.pb.h>
#include <octf/proto/packets.pb.h>

namespace octf {

class ConnectionTransceiver;
class MethodClient;
class InterfaceId;

/**
 * @typedef Unique pointer of Method
 */
typedef std::unique_ptr<MethodClient> MethodClientUniqueRef;

/**
 * @brief Class for pending method sent from client to service (plugin)
 *
 * Class gathers all elements needed to send method and keep method context
 */
class MethodClient {
public:
    /**
     * @param node Node for which this method is created
     * @param interface InterfaceId of method's interface
     * @param methodId ID of method within the interface
     * @param controller RPC controller
     * @param request Method input parameter
     * @param response Method output parameter
     * @param closure Method closure (completion callback)
     */
    MethodClient(const NodeWeakRef &node,
                 const InterfaceId &interface,
                 int methodId,
                 google::protobuf::RpcController *controller,
                 const google::protobuf::Message *request,
                 google::protobuf::Message *response,
                 google::protobuf::Closure *closure);

    virtual ~MethodClient();

    /**
     * @brief Prepares protocol buffer packet to be sent to the service
     *
     * @param[out] packet protocol buffer method packet
     *
     * @return operation result
     * @retval true packet prepared successfully
     * @retval false error while preparing packet
     */
    bool preparePacket(proto::Packet &packet);

    /**
     * @brief Fails Method
     *
     * @param reason Fail reason (error message)
     */
    void fail(const std::string &reason);

    /**
     * @brief Completes method
     *
     * @param response Method response received from service (plugin)
     */
    void complete(const std::string &response);

    /**
     * @brief Gets method sequence ID
     *
     * @param method Sequence ID
     */
    uint32_t getSid() const {
        return m_sid;
    }

    /**
     * @brief Sets method sequence ID
     *
     * @param sid Method sequence ID
     */
    void setSid(uint32_t sid) {
        m_sid = sid;
    }

    /**
     * @brief Checks if controller given as an argument is controller of this
     * method.
     *
     * @param controller Pointer to RpcController
     *
     * @retval true If given controller is the controller of this method
     * @retval false Otherwise
     */
    bool compareController(google::protobuf::RpcController *controller);

private:
    /**
     * InterfaceId
     */
    InterfaceId m_interface;
    /**
     * Method ID
     */
    int m_mId;
    /**
     * Node to which this method belongs
     */
    NodeWeakRef m_node;

    /**
     * Method sequence ID
     */
    uint32_t m_sid;

    /**
     * RPC controller
     */
    google::protobuf::RpcController *m_rpcController;

    /**
     * Method output parameter (request)
     */
    const google::protobuf::Message *m_request;

    /**
     * Method output parameter (response)
     */
    google::protobuf::Message *m_response;

    /**
     * Method closure (completion callback)
     */
    google::protobuf::Closure *m_closure;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_INTERNAL_METHODCLIENT_H
