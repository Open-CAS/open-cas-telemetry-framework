/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CONNECTION_H
#define SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CONNECTION_H

#include <octf/socket/ISocketConnection.h>
#include <octf/socket/SocketConfig.h>

namespace octf {
namespace unixsocket {

class Connection : public ISocketConnection {
public:
    Connection(int fd);
    virtual ~Connection();

    void close() override;

    bool send(const std::string &data) override;

    bool receive(std::string &data, size_t size = MAX_PACKET_SIZE) override;

    bool isActive() override;

private:
    int m_fd;
};

}  // namespace unixsocket
}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CONNECTION_H
