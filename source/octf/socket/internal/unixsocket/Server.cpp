/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <octf/socket/internal/unixsocket/Connection.h>
#include <octf/socket/internal/unixsocket/Server.h>

using namespace std;

namespace octf {
namespace unixsocket {

Server::Server(const string &address)
        : ISocketServer()
        , m_address(address)
        , m_fd(-1) {}

Server::~Server() {
    close();
}

bool Server::open() {
    int result;
    struct sockaddr_un unaddr;

    if (sizeof(unaddr.sun_path) < m_address.size() || m_address == "") {
        // TODO (mbarczak) Log error using logger
        // Address path too long, return error
        return false;
    }

    if (isActive()) {
        // Socket already open
        return false;
    }

    m_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_fd < 0) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        // Cannot open socket, return
        return false;
    }

    // Set permissions on socket (rw for user and group)
    if (fchmod(m_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) {
        m_fd = -1;
        ::close(m_fd);
        return false;
    }

    // Prepare address path
    memset(&unaddr, 0, sizeof(unaddr));
    unaddr.sun_family = AF_UNIX;
    strncpy(unaddr.sun_path, m_address.c_str(), sizeof(unaddr.sun_path) - 1);

    // TODO (jstencel) Fix correct unlinking on server closing.
    // As for now, just unlink it here as to enable server
    // running if the socket was opened and not closed by previous instance.
    // By default, on start SAS should check if this server
    // already exists and remove it if necessary.
    // TODO (trybicki) Add unit tests for checking, among others,
    // handling of opening socket multiple times
    unlink(unaddr.sun_path);
    result = bind(m_fd, (struct sockaddr *) &unaddr, sizeof(unaddr));
    if (result) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        ::close(m_fd);
        m_fd = -1;

        // Cannot bind to address
        return false;
    }

    return true;
}

SocketConnectionShRef Server::listen() {
    if (!isActive()) {
        // Socket not opened
        return NULL;
    }

    // TODO (mbarczak) implement limit of opened/accepted connection,
    // which shall be set in configuration

    int result = ::listen(m_fd, MAX_INCOMMING_CONN);
    if (result) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        // Error when listening, return NULL
        return NULL;
    }

    struct sockaddr_un clntUnixAddr;
    socklen_t clt_unaddr_len = 0;
    memset(&clntUnixAddr, 0, sizeof(struct sockaddr_un));

    int clntfd =
            accept(m_fd, (struct sockaddr *) &clntUnixAddr, &clt_unaddr_len);
    if (clntfd < 0) {
        // TODO (mbarczak) Log error using logger, use sock_errno
        // Cannot accept incoming connection
        close();
        return NULL;
    }

    return make_shared<Connection>(clntfd);
}

void Server::close() {
    // TODO (mbarczak) consider synchronization of connection closure

    if (!isActive()) {
        // Already closed
        return;
    }

    ::shutdown(m_fd, SHUT_RDWR);
    ::close(m_fd);
    m_fd = -1;

    if (::unlink(m_address.c_str())) {
        // TODO (mbarczak) Log warning using logger
    }
}

bool Server::isActive() {
    return m_fd > 0;
}

}  // namespace unixsocket
}  // namespace octf
