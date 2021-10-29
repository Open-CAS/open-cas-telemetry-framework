/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_ISOCKETSERVER_H
#define SOURCE_OCTF_SOCKET_ISOCKETSERVER_H

#include <octf/socket/ISocketConnection.h>

#include <memory>
#include <string>
#include <octf/utils/NonCopyable.h>

namespace octf {

struct SocketConfig;
class ISocketServer;

/** Server Socket Reference */
typedef std::shared_ptr<ISocketServer> SocketServerShRef;

/**
 * @interface ISocketServer
 *
 * @brief Definition for server side socket
 */
class ISocketServer : private NonCopyable {
public:
    ISocketServer() = default;
    virtual ~ISocketServer() = default;

    /**
     * @brief Open server socket
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    virtual bool open() = 0;

    /**
     * @brief Listen for incoming connection on server socket
     *
     * @note It can be blocking operation until a new connection has came
     *
     * @return Reference to the connection (shared pointer)
     */
    virtual SocketConnectionShRef listen() = 0;

    /**
     * @brief Close server socket and deny incoming calls
     */
    virtual void close() = 0;

    /**
     * @brief Check if server socket is active and working
     *
     * @retval true Server socket is active and working
     * @retval false Server socket is inactive, not working
     */
    virtual bool isActive() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_ISOCKETSERVER_H
