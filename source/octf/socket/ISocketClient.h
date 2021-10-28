/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_ISOCKETCLIENT_H
#define SOURCE_OCTF_SOCKET_ISOCKETCLIENT_H

#include <memory>
#include <string>

#include <octf/socket/ISocketConnection.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

struct SocketConfig;
class ISocketConnection;
class ISocketClient;

/** Client Socket Reference */
typedef std::shared_ptr<ISocketClient> SocketClientShRef;

/**
 * @interface ISocketClient
 *
 * @brief Class which defines interface for client side socket
 */
class ISocketClient : private NonCopyable {
public:
    ISocketClient() = default;
    virtual ~ISocketClient() = default;

    /**
     * @brief Start connection to the socket at specified address
     *
     * @return reference to socket connection (shared pointer)
     */
    virtual SocketConnectionShRef connect() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_ISOCKETCLIENT_H
