/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/socket/internal/unixsocket/Client.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <third_party/safestringlib.h>
#include <octf/socket/internal/unixsocket/Connection.h>

using namespace std;

namespace octf {
namespace unixsocket {

Client::Client(const string &address)
        : m_address(address) {}

Client::~Client() {}

SocketConnectionShRef Client::connect() {
    int clntfd, result, srvAddrLen;
    struct sockaddr_un srvAddr;

    // Initialize address
    memset_s(&srvAddr, sizeof(struct sockaddr_un), 0);

    // Open stream and get file descriptor
    clntfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (clntfd < 0) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        // Cannot create socket, return NULL
        return NULL;
    }

    // Setup socket address and connect to the server
    srvAddr.sun_family = AF_UNIX;
    strncpy(srvAddr.sun_path, m_address.c_str(), sizeof(srvAddr.sun_path) - 1);
    srvAddrLen = sizeof(srvAddr);
    result = ::connect(clntfd, (struct sockaddr *) &srvAddr, srvAddrLen);
    if (result < 0) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        // Cannot connect to the server, return NULL

        ::close(clntfd);
        return NULL;
    }

    return make_shared<Connection>(clntfd);
}

}  // namespace unixsocket
}  // namespace octf
