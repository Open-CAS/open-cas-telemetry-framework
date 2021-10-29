/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/communication/internal/ConnectionContext.h>

#include <octf/communication/CommunicationManagerServer.h>
#include <octf/communication/ICommunicationManager.h>
#include <octf/communication/Method.h>
#include <octf/communication/internal/MethodClient.h>
#include <octf/proto/packets.pb.h>
#include <octf/socket/ConnectionTransceiver.h>
#include <octf/socket/SocketConfig.h>
#include <octf/utils/Exception.h>

namespace octf {

ConnectionContext::ConnectionContext(ICommunicationManager *hndlr,
                                     const SocketConnectionShRef &conn)
        : std::enable_shared_from_this<ConnectionContext>()
        , IConnectionTransceiverListener()
        , NonCopyable()
        , m_hndlr(hndlr)
        , m_transceiver(new ConnectionTransceiver(conn, this))
        , m_conn(conn)
        , m_sidRef(0)
        , m_clntPendingMethods()
        , m_clntLock()
        , m_srvPendingMethods()
        , m_srvLock() {}

ConnectionContext::~ConnectionContext() {
    deactivate();
}

void ConnectionContext::activate() {
    m_transceiver->activate();
}

void ConnectionContext::deactivate() {
    m_transceiver->deactivate();

    cancelAllClientMethods("ERROR, Destruction");
}

void ConnectionContext::onDataPacket(const std::string &data) {
    proto::Packet packet;

    if (!packet.ParseFromString(data)) {
        // Cannot parse packet, did somebody hack us? Better close connection.
        m_transceiver->deactivate();
        return;
    }

    if (packet.has_methodresponse()) {
        completeClientMethod(packet.methodresponse());
        return;
    }

    if (packet.has_methodrequest()) {
        handleServerMethod(packet.mutable_methodrequest());
        return;
    }
}

void ConnectionContext::sendMethod(MethodClientUniqueRef method) {
    sid_t sid = getNextSid();
    method->setSid(sid);

    // Prepare Packet
    proto::Packet packet;
    if (!method->preparePacket(packet)) {
        method->fail("ERROR, cannot prepare packet");
        return;
    }

    {
        // Register method
        std::lock_guard<std::mutex> lock(m_clntLock);

        if (m_clntPendingMethods.size() > MAX_PENDING_METHODS) {
            method->fail("Maximum of pending methods reached");
            return;
        }
        if (m_clntPendingMethods.find(sid) != m_clntPendingMethods.end()) {
            method->fail("Method with given SID already registered");
            return;
        }

        m_clntPendingMethods.emplace(sid, std::move(method));
    }

    // TODO(mbarczak) Make data member of this class (it's protocol buffer
    // recommendation for avoiding heap fragmentation
    std::string data;

    // Send Method
    if (!packet.SerializeToString(&data)) {
        failClientMethod(sid, "ERROR, cannot serialize packet");
        return;
    }

    if (!m_transceiver->sendDataPacket(data)) {
        // Error while sending method, set method error and finish it
        failClientMethod(sid, "ERROR, cannot send packet");
        return;
    }

    // Now the method is traveling through galaxies almost at the speed of
    // light. Let's hope that we will get a reply.
}

void ConnectionContext::onDisconnection() {
    cancelAllClientMethods("ERROR, Disconnection");

    auto cntx = shared_from_this();
    m_hndlr->onConnectionExpiration(std::move(cntx));
}

void ConnectionContext::completeClientMethod(
        const proto::PacketMethodResponse &resp) {
    // first fetch method from the map of pending methods
    std::lock_guard<std::mutex> lock(m_clntLock);
    auto iter = m_clntPendingMethods.find(resp.ref_sid());
    if (iter != m_clntPendingMethods.end()) {
        // Alias name just for convenience of usage
        const MethodClientUniqueRef &method = iter->second;
        if (method) {
            if (!resp.has_result()) {
                // No result for this method, we have to return error
                method->fail("ERROR, Invalid response format");
            } else if (resp.result().code() != proto::RESULT_SUCCESS) {
                //  server/service side returned error
                if (resp.result().errormessage().empty()) {
                    method->fail("ERROR, server returned error");
                } else {
                    method->fail(resp.result().errormessage());
                }

            } else {
                // RESULT_SUCCESS
                method->complete(resp.response());
            }
        }
        m_clntPendingMethods.erase(iter);
    }
}

void ConnectionContext::failClientMethod(sid_t sid, const std::string &reason) {
    MethodClientUniqueRef method;

    // first fetch method from the map of pending methods
    std::lock_guard<std::mutex> lock(m_clntLock);
    auto iter = m_clntPendingMethods.find(sid);

    if (iter != m_clntPendingMethods.end()) {
        method = std::move(iter->second);

        if (method) {
            // finally fail method
            method->fail(reason);
        }
        m_clntPendingMethods.erase(iter);
    }
}

void ConnectionContext::cancelAllClientMethods(const std::string &reason) {
    std::lock_guard<std::mutex> lock(m_clntLock);

    while (m_clntPendingMethods.size()) {
        auto iter = m_clntPendingMethods.begin();
        MethodClientUniqueRef method = std::move(iter->second);

        if (method) {
            method->fail(reason);
        }

        m_clntPendingMethods.erase(iter);
    }
}

void ConnectionContext::cancelClientMethod(
        google::protobuf::RpcController *controller) {
    std::lock_guard<std::mutex> lock(m_clntLock);

    for (auto methodIter = m_clntPendingMethods.begin();
         methodIter != m_clntPendingMethods.end(); ++methodIter) {
        MethodClient *method = methodIter->second.get();

        if (method->compareController(controller)) {
            method->fail("Cancel of method requested.");
            method = nullptr;
            m_clntPendingMethods.erase(methodIter);
            // Only one method is expected to have given controller.
            break;
        }
    }
}

void ConnectionContext::sendResponse(Method *method) {
    // find method on pending server methods map
    MethodShRef sMethod;
    proto::Packet packet;

    {
        std::lock_guard<std::mutex> lock(m_srvLock);

        auto iter = m_srvPendingMethods.find(method->getSequenceId());
        if (iter == m_srvPendingMethods.end()) {
            // No method found with this sid, just stop processing
            return;
        }

        // Grub shared pointer of method
        sMethod = iter->second;

        // Remove method from map
        m_srvPendingMethods.erase(iter);

        // Sanity check, completing method shall be the same like kept by
        // sheared pointer
        if (method != sMethod.get()) {
            // Oops, wrong program state, it's better to log it and abort
            // (TODO) mbarczak log critical error
            abort();
        }
    }

    method->prepareResponsePacket(packet);

    // TODO(mbarczak) Make data member of this class (it's protocol buffer
    // recommendation for avoiding heap fragmentation
    std::string data;

    if (packet.SerializeToString(&data)) {
        m_transceiver->sendDataPacket(data);
    }
}

void ConnectionContext::handleServerMethod(
        proto::PacketMethodRequest *request) {
    auto conn = shared_from_this();
    auto srvMethod = std::make_shared<Method>(conn);

    if (!srvMethod->prepareMethod(request)) {
        // Problem with packet, does somebody hack us? Better close connection.
        m_transceiver->deactivate();
        return;
    }

    // Register server pending request
    {
        std::lock_guard<std::mutex> lock(m_srvLock);

        // Check if is there a method with the same SID
        if (m_srvPendingMethods.find(srvMethod->getSequenceId()) !=
            m_srvPendingMethods.end()) {
            // Already method with this SID registered
            srvMethod->fail("Method with given SID already registered");

            // Did somebody hack us? Better close the connection.
            m_transceiver->deactivate();
            return;
        }

        // Check if limit of pending methods is reached
        if (m_srvPendingMethods.size() > MAX_PENDING_METHODS) {
            srvMethod->fail("Maximum of pending methods reached");
            return;
        }

        // OK, no such method with this SID, we can register this request
        m_srvPendingMethods.insert(std::pair<sid_t, MethodShRef>(
                srvMethod->getSequenceId(), srvMethod));
    }

    // Now we can handle request
    m_hndlr->handleServerMethod(srvMethod);
}

ConnectionContext::sid_t ConnectionContext::getNextSid() {
    sid_t newSid = m_sidRef++;

    // Check if sid already in use
    std::lock_guard<std::mutex> lock(m_clntLock);

    while (m_clntPendingMethods.find(newSid) != m_clntPendingMethods.end()) {
        newSid = m_sidRef++;
    }
    return newSid;
}

}  // namespace octf
