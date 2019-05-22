/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_INTERNAL_RPCPROXY_H
#define SOURCE_OCTF_COMMUNICATION_INTERNAL_RPCPROXY_H

#include <mutex>
#include <octf/communication/RpcChannelImpl.h>
#include <octf/communication/RpcControllerImpl.h>
#include <octf/communication/internal/MethodClient.h>

namespace octf {

/**
 * @brief RPC channel proxy.
 *
 * The RPC channel is responsible for passing client methods to the active
 * connection context.
 *
 * Each shadow node contains RPC channel which points to a RPC proxy.
 * During a method call following sequence take place:
 * A method call on client interface is performed -> RPC channel is involved ->
 * the client method is created -> the client method is passed to RPC proxy ->
 * RPC proxy calls sending method on active connection context.
 */
class RpcProxy {
public:
    RpcProxy();
    virtual ~RpcProxy();

    /**
     * @brief Set connection on this RPC channel.
     *
     * Method will be forwarded into set connection context
     *
     * @param connCntx Connection context to be set
     */
    void setConnectionContext(const ConnectionContextShRef &connCntx);

    /**
     * @brief clear (invalidate) connection context
     *
     * It is called in case of disconnection
     */
    void clearConnectionContext();

    /**
     * @brief send client method to be executed by service or plugin
     *
     * @param method Client method to be executed
     */
    void sendMethod(MethodClientUniqueRef method);

    void cancelClientMethod(google::protobuf::RpcController *controller);

private:
    void waitForConnectionContext();

private:
    /**
     * Lock for connection context exchanging
     */
    std::mutex m_lock;

    /**
     * Connection context to which methods to be sent will be forwarded
     */
    ConnectionContextShRef m_connCntx;

    /**
     * Conditional variable for signaling if connection context is available
     */
    std::condition_variable m_connCntxAvailable;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_INTERNAL_RPCPROXY_H
