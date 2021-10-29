/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/interface/InterfaceIdentificationImpl.h>

#include <octf/utils/Exception.h>
#include <octf/utils/ProtoConverter.h>

namespace octf {

void InterfaceIdentificationImpl::getNodeId(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::NodeId *response,
        ::google::protobuf::Closure *done) {
    // Request parameter is Void and not used
    (void) request;

    try {
        const NodeId &node = m_owner->getNodeId();
        protoconverter::convertNodeId(response, node);
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    // Response (command) ready, send it back
    // It's done both for sending correct response and informing
    // about some failure during execution of command
    done->Run();
}

void InterfaceIdentificationImpl::getChildren(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::ChildrenList *response,
        ::google::protobuf::Closure *done) {
    // Request parameter is Void and not used
    (void) request;

    NodesIdList list;

    try {
        bool result = m_owner->getChildrenIdList(list);

        if (!result) {
            throw Exception("Error getting children list.");
        }

        for (const auto &id : list) {
            protoconverter::convertNodeId(response->add_child(), id);
        }

    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    // Response (command) ready, send it back
    // It's done both for sending correct response and informing
    // about some failure during execution of command
    done->Run();
}

void InterfaceIdentificationImpl::getInterfaces(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void *request,
        ::octf::proto::InterfaceList *response,
        ::google::protobuf::Closure *done) {
    // Request parameter is Void and not used
    (void) request;

    InterfacesIdList childrenList;

    try {
        bool result = m_owner->getInterfacesIdList(childrenList);

        if (!result) {
            throw Exception("Error getting interfaces list.");
        }

        for (const auto &interfaceId : childrenList) {
            protoconverter::convertInterfaceId(response->add_interface(),
                                               interfaceId);
        }
    } catch (Exception &e) {
        controller->SetFailed(e.what());
    }

    // Response (command) ready, send it back
    // It's done both for sending correct response and informing
    // about some failure during execution of command
    done->Run();
}
}  // namespace octf
