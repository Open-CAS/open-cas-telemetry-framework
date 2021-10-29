/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_ICOMMUNICATIONMANAGER_H
#define SOURCE_OCTF_COMMUNICATION_ICOMMUNICATIONMANAGER_H

#include <octf/communication/IMethodHandler.h>
#include <octf/communication/RpcChannelImpl.h>
#include <octf/socket/ISocketListener.h>

namespace octf {

/**
 * @interface ICommunicationManager
 *
 * @brief The communication manager interface for informing about various events
 * like connection expiration, new method to be handled, etc...
 *
 * The communication manager is used by node and it provides possibility of
 * starting either server or client socket. Upon new incoming connection the
 * manager creates ConnectionContext. The connection context informs the
 * communication manager via this interface about various events.
 */
class ICommunicationManager : public ISocketListener {
public:
    ICommunicationManager() = default;
    virtual ~ICommunicationManager() = default;

    /**
     * @brief Connection Context expiration event
     *
     * Expiration can take place for instance due to closing socket at server
     * or client side. Upon this event communication is not possible, and all
     * resources associated with given communication context shall be freed.
     *
     * @param context Connection context which has expired
     */
    virtual void onConnectionExpiration(ConnectionContextShRef &&context) = 0;

    /**
     * @brief Event of server method to be executed
     *
     * @param method Server method to be executed
     */
    virtual void handleServerMethod(const MethodShRef &method) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_ICOMMUNICATIONMANAGER_H
