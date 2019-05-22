/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_SOCKETFACTORY_H
#define SOURCE_OCTF_SOCKET_SOCKETFACTORY_H

#include <memory>
#include <string>

#include <octf/socket/ISocketClient.h>
#include <octf/socket/ISocketServer.h>
#include <octf/socket/SocketConfig.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * Utilities which shall be used to create given Socket implementation
 */
class SocketFactory : private NonCopyable {
public:
    SocketFactory();
    virtual ~SocketFactory();

    /**
     * @brief Create server socket
     *
     * @param address Address of server
     * @param implementation Type of socket
     *
     * @return Server socket reference (shared pointer)
     */
    static SocketServerShRef createServer(const std::string &address,
                                          SocketImplementation implementation);

    /**
     * @brief Create client socket
     *
     * @param address Address of server
     * @param implementation Socket implementation
     *
     * @return Client socket reference (shared pointer)
     */
    static SocketClientShRef createClient(const std::string &address,
                                          SocketImplementation implementation);
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_SOCKETFACTORY_H
