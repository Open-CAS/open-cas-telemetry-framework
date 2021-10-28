/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_SERVER_H
#define SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_SERVER_H

#include <string>

#include <octf/socket/ISocketServer.h>
#include <octf/socket/SocketConfig.h>

namespace octf {
namespace unixsocket {

class Server : public ISocketServer {
public:
    Server(const std::string &address);
    virtual ~Server();

    bool open() override;

    SocketConnectionShRef listen() override;

    void close() override;

    bool isActive() override;

private:
    /**
     * Socket address
     */
    std::string m_address;

    /**
     * File descriptor of UNIX socket
     */
    int m_fd;

    /**
     * Maximum of incoming connections waiting for accept
     */
    static constexpr int MAX_INCOMMING_CONN = 32;
};

}  // namespace unixsocket
}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_SERVER_H
