/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_NODESHADOW_H
#define SOURCE_OCTF_NODE_NODESHADOW_H

#include <octf/communication/Call.h>
#include <octf/communication/RpcChannelImpl.h>
#include <octf/node/NodeGeneric.h>
#include <octf/proto/InterfaceIdentification.pb.h>
#include <octf/utils/Exception.h>

namespace octf {

/**
 * @brief Base class for shadow nodes
 *
 * Shadow node is created at client side and reflects to base node created
 * at server side
 */
class NodeShadow : public NodeGeneric {
public:
    NodeShadow(const NodeId &id);
    virtual ~NodeShadow();

    bool initCommon() override;

    /**
     * @brief Get Identification interface
     *
     * @return Shared pointer to identification interface
     */
    std::shared_ptr<proto::InterfaceIdentification>
    getIdentificationInterface() {
        return findInterface<proto::InterfaceIdentification_Stub>();
    }

    /**
     * @brief Creates and adds an interface to the list.
     *
     * @tparam T Class type of interface to be created and added
     *
     * @return Shared pointer to the interface
     *
     * @retval Valid shared pointer indicates success of operation
     *
     * @retval Null shared pointer indicates operation failure
     * (because of some error or already existing interface with the same
     * InterfaceId on the list)
     */
    template <typename T>
    std::shared_ptr<T> createInterface() {
        try {
            InterfaceId id(T::descriptor());
            if (hasInterface(id)) {
                return nullptr;
            }

            if (!m_rpcChannel) {
                // RPC channel not initialized, cannot add interface
                return nullptr;
            }

            auto interface = std::make_shared<T>(m_rpcChannel.get());

            if (NodeGeneric::addInterface(interface)) {
                return interface;
            }

            return nullptr;

        } catch (Exception &) {
            return nullptr;
        }
    }

    bool addChild(NodeShRef child) override;

    virtual const RpcChannelShRef &getRpcChannel() {
        return m_rpcChannel;
    }

    void setRpcChannel(const RpcChannelShRef &rpcChannel) {
        m_rpcChannel = rpcChannel;
    }

private:
    /**
     * RPC channel
     */
    RpcChannelShRef m_rpcChannel;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODESHADOW_H
