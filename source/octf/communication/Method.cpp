/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/communication/Method.h>

#include <octf/communication/internal/ConnectionContext.h>
#include <octf/utils/Exception.h>

namespace octf {

Method::Method(std::weak_ptr<ConnectionContext> connCntx)
        : Closure()
        , m_connCntx(connCntx)
        , m_path()
        , m_interfaceId()
        , m_methodID()
        , m_sequenceId()
        , m_requestPayload()
        , m_input()
        , m_output()
        , m_rpcControler() {}

bool Method::prepareMethod(proto::PacketMethodRequest *request) {
    // build path
    for (int i = 0; i < request->path().node_size(); i++) {
        try {
            m_path.emplace_back(NodeId(request->path().node(i)));
        } catch (InvalidParameterException &) {
            // Invalid Node ID and cannot build node path, end processing
            return false;
        }
    }

    if (!m_path.size()) {
        return false;
    }

    // set interface id
    m_interfaceId = request->interface();

    // set method id
    m_methodID = request->mid();

    // set sequence id
    m_sequenceId = request->sid();

    // set request payload
    m_requestPayload = request->request();

    return true;
}

void Method::prepareResponsePacket(proto::Packet &packet) {
    proto::PacketMethodResponse *response = packet.mutable_methodresponse();

    response->set_mid(m_methodID);
    response->set_ref_sid(m_sequenceId);

    if (m_rpcControler.Failed()) {
        response->mutable_result()->set_code(proto::RESULT_ERROR);
        response->mutable_result()->set_errormessage(
                m_rpcControler.ErrorText());
    } else {
        response->mutable_result()->set_code(proto::RESULT_SUCCESS);

        // Serialize response payload
        if (!m_output->SerializeToString(response->mutable_response())) {
            // Cannot serialize response, mark error
            response->mutable_result()->set_code(proto::RESULT_ERROR);
            response->mutable_result()->set_errormessage(
                    "Cannot serialize response");
        }
    }
}

void Method::Run() {
    // First check if connection context still exist
    auto connCntx = m_connCntx.lock();
    if (!connCntx) {
        // No connection context (probably had been disconnected)
        // Don't sent response
        return;
    }

    connCntx->sendResponse(this);
}

bool Method::prepareParameters(const InterfaceShRef &interface) {
    // Get method descriptor
    auto mDesc = getMethodDescriptor(interface);
    if (!mDesc) {
        // Cannot get method descriptor
        return false;
    }

    // Get prototypes
    auto &inPrototype = interface->GetRequestPrototype(mDesc);
    auto &outPrototype = interface->GetResponsePrototype(mDesc);

    // Allocate input and output parameters and store them into smart pointers
    m_input.reset(inPrototype.New());
    m_output.reset(outPrototype.New());

    if (!m_input || !m_output) {
        return false;
    }

    // Now parse request payload into input parameter
    if (!m_input->ParseFromString(m_requestPayload)) {
        // Cannot parse payload into input parameter
        return false;
    }

    return true;
}

const google::protobuf::MethodDescriptor *Method::getMethodDescriptor(
        const InterfaceShRef &interface) {
    if (m_methodID > interface->GetDescriptor()->method_count()) {
        return nullptr;
    }

    // Get method descriptor
    return interface->GetDescriptor()->method(m_methodID);
}

}  // namespace octf
