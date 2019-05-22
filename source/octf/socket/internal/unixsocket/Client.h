/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CLIENT_H
#define SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CLIENT_H

#include <string>

#include <octf/socket/ISocketClient.h>
#include <octf/socket/SocketConfig.h>

namespace octf {
namespace unixsocket {

class Client : public ISocketClient {
public:
    Client(const std::string &address);
    virtual ~Client();

    SocketConnectionShRef connect() override;

private:
    /**
     * Socket address
     */
    std::string m_address;
};

}  // namespace unixsocket
}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_UNIXSOCKET_CLIENT_H
