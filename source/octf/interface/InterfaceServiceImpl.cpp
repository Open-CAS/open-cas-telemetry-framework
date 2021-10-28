/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <memory>
#include <octf/interface/InterfaceServiceImpl.h>
#include <octf/plugin/NodePluginShadow.h>
#include <octf/utils/Exception.h>

namespace octf {

void InterfaceServiceImpl::registerPlugin(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::NodeId *request,
        ::octf::proto::Void __attribute__((__unused__)) * response,
        ::google::protobuf::Closure *done) {
    try {
        NodeId pluginId(*request);

        std::shared_ptr<INode> existingNode = m_owner->getChild(pluginId);
        if (existingNode) {
            throw Exception("Plugin with given ID is already registered.");
        } else {
            // Create and initialize new node with specified ID
            std::shared_ptr<NodePluginShadow> pluginNode =
                    std::make_shared<NodePluginShadow>(pluginId);
            if (!pluginNode->init()) {
                throw Exception("Cannot initialize plugin node.");
            }
            if (m_owner->addChild(pluginNode)) {
                pluginNode->setState(PluginState::CONNECTED);
            } else {
                // NodeId probably is already used (or some other error occured
                // during adding plugin's node as a child to service), deny
                // plugin registration
                throw Exception("Adding plugin as a child failed.");
            }
        }
    } catch (Exception &e) {
        controller->SetFailed("Register operation error. " + e.getMessage());
    }
    // Method's execution finished, send the response back
    done->Run();
}

void InterfaceServiceImpl::unregisterPlugin(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::NodeId *request,
        ::octf::proto::Void __attribute__((__unused__)) * response,
        ::google::protobuf::Closure *done) {
    try {
        NodeId pluginId(*request);
        bool result = m_owner->removeChild(pluginId);
        if (!result) {
            throw Exception("Error on removing child.");
        }
    } catch (Exception &e) {
        controller->SetFailed("Unregister operation unexpected error. " +
                              e.getMessage());
    }
    done->Run();
}

void InterfaceServiceImpl::connectPlugin(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::NodeId *request,
        ::octf::proto::Void __attribute__((__unused__)) * response,
        ::google::protobuf::Closure *done) {
    try {
        NodeId pluginId(*request);

        std::shared_ptr<INode> existingNode = m_owner->getChild(pluginId);
        if (existingNode) {
            // Node with specified ID already exists, check if it's plugin type
            std::shared_ptr<NodePluginShadow> pluginNode =
                    std::dynamic_pointer_cast<NodePluginShadow>(existingNode);

            if (pluginNode &&
                pluginNode->getState() != PluginState::CONNECTED) {
                pluginNode->setState(PluginState::CONNECTED);
                pluginNode->setError("");
            } else {
                throw Exception(
                        "Connection operation unavailable for the plugin.");
            }
        } else {
            throw Exception("Plugin with specified ID does not exist.");
        }

    } catch (Exception &e) {
        controller->SetFailed("Register operation error. " + e.getMessage());
    }
    // Method's execution finished, send the response back
    done->Run();
}

void InterfaceServiceImpl::disconnectPlugin(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::NodeId *request,
        ::octf::proto::Void __attribute__((__unused__)) * response,
        ::google::protobuf::Closure *done) {
    try {
        NodeId pluginId(*request);
        std::shared_ptr<NodePluginShadow> pluginNode =
                std::dynamic_pointer_cast<NodePluginShadow>(
                        m_owner->getChild(pluginId));
        if (!pluginNode) {
            controller->SetFailed("Nonexistent plugin. ");
        } else {
            pluginNode->setState(PluginState::DISCONNECTED);
        }
    } catch (Exception &e) {
        controller->SetFailed("Disconnect operation unexpected error. " +
                              e.getMessage());
    }
    done->Run();
}

void InterfaceServiceImpl::listPlugins(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::Void __attribute__((__unused__)) * request,
        ::octf::proto::PluginsList *response,
        ::google::protobuf::Closure *done) {
    try {
        // Dummy casting to avoid compilation warning
        (void) request;

        NodesIdList nodesIds;
        m_owner->getChildrenIdList(nodesIds);

        for (auto nodeId : nodesIds) {
            std::shared_ptr<NodePluginShadow> node =
                    std::dynamic_pointer_cast<NodePluginShadow>(
                            m_owner->getChild(nodeId));
            if (node) {
                proto::PluginStatus *pluginStatus = response->add_status();

                pluginStatus->mutable_id()->set_id(nodeId.getId());

                const std::string &desc = node->getDescription();
                pluginStatus->set_description(desc.c_str());

                const std::string &error = node->getError();
                pluginStatus->set_error(error.c_str());

                PluginState state = node->getState();
                proto::PluginStatus_State protoPluginState =
                        proto::PluginStatus_State_UNDEFINED;
                switch (state) {
                case PluginState::CONNECTED:
                    protoPluginState = proto::PluginStatus_State_CONNECTED;
                    break;
                case PluginState::DISCONNECTED:
                    protoPluginState = proto::PluginStatus_State_DISCONNECTED;
                    break;
                case PluginState::ERROR:
                    protoPluginState = proto::PluginStatus_State_ERROR;
                    break;
                case PluginState::UNDEFINED:
                    protoPluginState = proto::PluginStatus_State_UNDEFINED;
                    break;
                default:
                    throw Exception("Plugin State error.");
                    break;
                }
                pluginStatus->set_state(protoPluginState);
            }
        }
    } catch (Exception &e) {
        controller->SetFailed("Error in plugins' listing. " + e.getMessage());
    }
    done->Run();
}

}  // namespace octf
