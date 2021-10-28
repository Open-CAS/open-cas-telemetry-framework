/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXT_H
#define SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXT_H

#include <google/protobuf/service.h>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <octf/communication/IMethodHandler.h>
#include <octf/communication/internal/MethodClient.h>
#include <octf/node/INode.h>
#include <octf/proto/packets.pb.h>
#include <octf/socket/IConnectionTransceiverListener.h>
#include <octf/socket/ISocketConnection.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

struct SocketConfig;
class ICommunicationManager;
class ConnectionTransceiver;
class ConnectionContext;
class MethodClient;
class Method;

/**
 * @brief Connection context responsible for handling one socket connection
 */
class ConnectionContext
        : public std::enable_shared_from_this<ConnectionContext>,
          public IConnectionTransceiverListener,
          private NonCopyable {
public:
    /**
     * @param hndlr Connection context handler
     * @param conn Socket connection reference
     */
    ConnectionContext(ICommunicationManager *hndlr,
                      const SocketConnectionShRef &conn);

    virtual ~ConnectionContext();

    void sendMethod(MethodClientUniqueRef method);

    void sendResponse(Method *method);

    void cancelClientMethod(google::protobuf::RpcController *controller);

    /**
     * @brief Activates connection context
     *
     * After activation, connection context starts receiving packets
     */
    void activate();

    /**
     * Deactivates connection context
     *
     * After de-activation, connection context stops receiving packets.
     */
    void deactivate();

private:
    /**
     * @typedef Method sequence id typedef
     */
    typedef uint32_t sid_t;

    void onDataPacket(const std::string &data) override;

    void onDisconnection() override;

    void completeClientMethod(const proto::PacketMethodResponse &resp);

    void failClientMethod(sid_t sid, const std::string &reason);

    void cancelAllClientMethods(const std::string &reason);

    void handleServerMethod(proto::PacketMethodRequest *request);

    sid_t getNextSid();

private:
    /**
     * Owner of this context
     */
    ICommunicationManager *m_hndlr;

    /**
     * Connection transceiver
     */
    std::unique_ptr<ConnectionTransceiver> m_transceiver;

    /**
     * Connection for which this context is created
     */
    const SocketConnectionShRef m_conn;

    /**
     * Sequence ID reference for new methods
     */
    std::atomic<sid_t> m_sidRef;

    /**
     * Map for storing pending methods sent by client side
     */
    std::map<sid_t, MethodClientUniqueRef> m_clntPendingMethods;

    /**
     * Lock for synchronizing map of client pending methods
     */
    std::mutex m_clntLock;

    /**
     * Map for storing pending methods to be executed by service or plugin side
     */
    std::map<sid_t, MethodShRef> m_srvPendingMethods;

    /**
     * Lock for synchronizing map of client pending methods
     */
    std::mutex m_srvLock;

    static constexpr int MAX_PENDING_METHODS = 10000;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_INTERNAL_CONNECTIONCONTEXT_H
