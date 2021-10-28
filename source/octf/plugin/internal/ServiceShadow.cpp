/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <functional>
#include <octf/node/events/NodeEventHandlerFunction.h>
#include <octf/node/events/NodeShadowEvents.h>
#include <octf/plugin/internal/ServiceShadow.h>
#include <octf/proto/InterfaceService.pb.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtoConverter.h>

namespace octf {

ServiceShadow::ServiceShadow()
        : NodePluginShadow(NodeId("octf"))
        , m_autoRegistrationHandlers() {}

ServiceShadow::~ServiceShadow() {}

bool ServiceShadow::initCustom() {
    if (getServiceInterfaces()) {
        return true;
    } else {
        return false;
    }
}

void ServiceShadow::deinitCustom() {}

bool ServiceShadow::registerPlugin(const NodeId &id) {
    auto serviceInterface = getServiceInterfaces();

    Call<proto::NodeId, proto::Void> call(this);
    protoconverter::convertNodeId(call.getInput().get(), id);

    serviceInterface->registerPlugin(&call, call.getInput().get(),
                                     call.getOutput().get(), &call);
    call.waitFor();

    if (call.Failed()) {
        log::cout << "Plugin " << id.getId() << " registration failed."
                  << std::endl;
        return false;
    } else {
        log::cout << "Plugin " << id.getId() << " registered successfully."
                  << std::endl;
        return true;
    }
}

bool ServiceShadow::unregisterPlugin(const NodeId &id) {
    auto serviceInterface = getServiceInterfaces();

    Call<proto::NodeId, proto::Void> call(this);
    protoconverter::convertNodeId(call.getInput().get(), id);

    serviceInterface->unregisterPlugin(&call, call.getInput().get(),
                                       call.getOutput().get(), &call);
    call.waitFor();

    if (call.Failed()) {
        log::cerr << "Plugin " << id.getId() << " unregistration failed."
                  << std::endl;
        return false;
    } else {
        log::cout << "Plugin " << id.getId() << " unregistered successfully."
                  << std::endl;
        return true;
    }
}

void ServiceShadow::enablePluginAutoRegistration(const NodeId &id) {
    auto iter = m_autoRegistrationHandlers.find(id);
    if (iter != m_autoRegistrationHandlers.end()) {
        // Plugin with this ID already enabled for auto registration
        return;
    }

    auto fn = [this, id](NodeEventShRef event) {
        if (event->isInstanceOf<NodeShadowEventOnAttach>()) {
            registerPlugin(id);
        } else if (event->isInstanceOf<NodeShadowEventOnDetach>()) {
            log::cout << "Plugin " << id.getId() << " detached." << std::endl;
        }
    };

    auto handler = std::make_shared<NodeEventHandlerFunction>(fn);

    handler->getEventFilter().enable<NodeShadowEventOnAttach>();
    handler->getEventFilter().enable<NodeShadowEventOnDetach>();

    getEventDispatcher().registerEventHandler(handler);

    m_autoRegistrationHandlers.emplace(id, handler);
}

void ServiceShadow::disablePluginAutoRegistration(const NodeId &id) {
    auto iter = m_autoRegistrationHandlers.find(id);
    if (iter == m_autoRegistrationHandlers.end()) {
        // Plugin with this ID already disabled for auto registration
        return;
    }

    getEventDispatcher().unregisterEventHandler(iter->second);
    m_autoRegistrationHandlers.erase(iter);

    // At the end unregister plugin
    unregisterPlugin(id);
}

std::shared_ptr<proto::InterfaceService_Stub>
ServiceShadow::getServiceInterfaces() {
    auto serviceInterface = findInterface<proto::InterfaceService_Stub>();

    if (!serviceInterface) {
        serviceInterface = createInterface<proto::InterfaceService_Stub>();
    }

    return serviceInterface;
}

}  // namespace octf
