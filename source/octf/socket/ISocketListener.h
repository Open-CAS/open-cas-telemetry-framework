/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_ISOCKETLISTENER_H
#define SOURCE_OCTF_SOCKET_ISOCKETLISTENER_H

#include <string>
#include <octf/socket/ISocketConnection.h>

namespace octf {

/**
 * @interface ISocketListener
 *
 * @brief Socket Listener Interface
 *
 * Through this interface user will be informed about new incoming connection,
 */
class ISocketListener {
public:
    ISocketListener() {}
    virtual ~ISocketListener() {}

    /**
     * @brief On new connection event
     *
     * @param[in] conn Reference to the new connection
     */
    virtual void onConnection(const SocketConnectionShRef &conn) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_ISOCKETLISTENER_H
