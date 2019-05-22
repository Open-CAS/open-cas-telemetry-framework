/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_RPCCHANNELIMPL_H
#define SOURCE_OCTF_COMMUNICATION_RPCCHANNELIMPL_H

#include <google/protobuf/service.h>
#include <mutex>
#include <octf/communication/Call.h>
#include <octf/interface/InterfaceId.h>
#include <octf/node/INode.h>
#include <octf/socket/SocketConfig.h>

namespace octf {

class ConnectionContext;
class RpcChannelImpl;
class RpcProxy;

/**
 * @typedef Shared pointer for connection context
 */
typedef std::shared_ptr<ConnectionContext> ConnectionContextShRef;

/**
 * @typedef RPC channel reference (shared pointer)
 */
typedef std::shared_ptr<RpcChannelImpl> RpcChannelShRef;

/**
 * @typedef RPC channel proxy reference (shared pointer)
 *
 * The RPC proxy is responsible for tunneling a method to be executed
 * into active connection.
 */
typedef std::shared_ptr<RpcProxy> RpcProxyShRef;

/**
 * @brief Protocol buffer RPC Channel implementation
 *
 * The main purpose of this class is handling method call and forward it into
 * RPC proxy. Then protocol buffer will be prepared and send
 * to service (plugin)
 */
class RpcChannelImpl : public google::protobuf::RpcChannel {
public:
    /**
     * @param node Node for which RPC channel is created
     * @param rpcProxy RPC proxy
     */
    RpcChannelImpl(const NodeShRef &node, const RpcProxyShRef &rpcProxy);

    virtual ~RpcChannelImpl();

    /**
     * @brief Invokes remote call of method described with protobuf way.
     *
     * @param method Method descriptor. If nullptr is passed, it resets
     * (cancels) last method called with given controller.
     * @param controller Controller dedicated for particular method. Each method
     * call should have its own controller associated.
     * @param request Message containing all input parameters.
     * @param response Message for receiving output parameters.
     * @param closure Object providing Run() method, to be called on method
     * completion.
     *
     * Implements protobuf RpcChannel abstract method. Typically is called
     * by auto-generated Interface (Service) Stub.
     */
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *closure) override;

    /**
     * @brief Invokes remote call of method.
     *
     * This method enables calling remotely a method without direct
     * access to the Interface Stub which it belongs to.
     *
     * @param interface Unique identifier of the method's Interface
     * @param methodId Identifier of the method inside of Interface
     * @param controller Protobuf controller
     * @param request Message containing input values
     * @param[out] response Message for receiving method's output
     * @param closure Protobuf closure
     */
    void genericCallMethod(const InterfaceId &interface,
                           const int methodId,
                           google::protobuf::RpcController *controller,
                           const google::protobuf::Message *request,
                           google::protobuf::Message *response,
                           google::protobuf::Closure *closure);
    /**
     * @brief Invokes remote call of method for CallGeneric.
     *
     * @param interface Unique identifier of the method's Interface
     * @param methodId Identifier of the method inside of Interface
     * @param[in, out] call Generic call which corresponds to input parameter,
     * output parameter, and RPC controller and method closure
     * (completion callback)
     */
    inline void genericCallMethod(const InterfaceId &interface,
                                  const int methodId,
                                  CallGeneric &call) {
        genericCallMethod(interface, methodId, &call, call.getInput().get(),
                          call.getOutput().get(), &call);
    }

    /**
     * @brief Gets RPC proxy
     *
     * @return RPC proxy
     */
    RpcProxyShRef getRpcProxy() const {
        return m_rpcProxy;
    }

private:
    /**
     * Node reference to which this RPC Channel belongs
     */
    NodeWeakRef m_node;

    /**
     * RPC proxy (communication tunnel for sending methods)
     */
    RpcProxyShRef m_rpcProxy;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_RPCCHANNELIMPL_H
