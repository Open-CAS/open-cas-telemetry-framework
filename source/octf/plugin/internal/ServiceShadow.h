/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_PLUGIN_INTERNAL_SERVICESHADOW_H
#define SOURCE_OCTF_PLUGIN_INTERNAL_SERVICESHADOW_H

#include <map>
#include <octf/plugin/NodePluginShadow.h>
#include <octf/proto/InterfaceService.pb.h>

namespace octf {

/**
 * @brief Service shadow class
 *
 * This class represents the shadow of service node. It can be used to execute a
 * method on a service's interface. In addition this class provides a plugin
 * auto registration functionality if the plugin enables it.
 */
class ServiceShadow : public NodePluginShadow {
public:
    ServiceShadow();
    virtual ~ServiceShadow();

    bool initCustom() override;

    void deinitCustom() override;

    /**
     * @brief Registers plugin into storage analytics service
     *
     * @retval true Plugin registered successfully
     * @retval false Plugin registered failure
     */
    bool registerPlugin(const NodeId &id);

    /**
     * @brief Unregisters plugin from Storage analytics service
     *
     * @retval true Plugin unregistered successfully
     * @retval false Plugin unregistered failure
     */
    bool unregisterPlugin(const NodeId &id);

    /**
     * @brief Enables plugin auto-registration
     *
     * The auto-registration process consists of plugin registration to the
     * service whenever connection between the plugin and the service is
     * established.
     *
     * @param id Id of plugin for which performs auto-registration
     */
    void enablePluginAutoRegistration(const NodeId &id);

    /**
     * @brief Disables plugin auto-registration
     *
     * @param id Id of plugin for which disables auto-registration
     */
    void disablePluginAutoRegistration(const NodeId &id);

private:
    std::shared_ptr<proto::InterfaceService_Stub> getServiceInterfaces();

private:
    /**
     * Map of plugin auto registration handlers
     */
    std::map<NodeId, NodeEventHandlerShRef> m_autoRegistrationHandlers;
};

}  // namespace octf

#endif  // SOURCE_OCTF_PLUGIN_INTERNAL_SERVICESHADOW_H
