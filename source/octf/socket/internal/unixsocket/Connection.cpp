/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/socket/internal/unixsocket/Connection.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

namespace octf {
namespace unixsocket {

Connection::Connection(int fd)
        : m_fd(fd) {}

Connection::~Connection() {
    close();
}

void Connection::close() {
    // TODO (mbarczak) consider synchronization of connection closure

    if (isActive()) {
        ::shutdown(m_fd, SHUT_RDWR);
        ::close(m_fd);
        m_fd = -1;
    }
}

bool Connection::send(const string &data) {
    if (!isActive()) {
        // Connection is not active, no valid file descriptor
        return false;
    }

    size_t sent = 0, to_send = data.size();

    while (to_send) {
        int length = min(MAX_PACKET_SIZE, to_send);

        int result = ::send(m_fd, data.c_str() + sent, length, MSG_NOSIGNAL);
        if (result > 0) {
            if (result > length) {
                // This should never happen, close it
                close();
                return false;
            }

            to_send -= result;
            sent += result;

        } else {
            // Error when sending, close connection
            close();
            return false;
        }
    }

    return true;
}

bool Connection::receive(string &data, size_t size) {
    if (!isActive()) {
        // Not valid file descriptor
        return false;
    }

    if (size > MAX_PACKET_SIZE) {
        // TODO (mbarczak) Log exception
        return false;
    }

    int to_receive = size;

    // prepare buffer with given size
    data.resize(to_receive);

    // receive bytes
    int result = ::recv(m_fd, &data[0], to_receive, 0);

    // Check the result and shrink buffer in case when received
    // number of bytes is smaller than requested

    if (result < 0) {
        // TODO (mbarczak) Log exception
        return false;
    } else if (result == 0) {
        // According to the documentation peer side closed connection,
        // do the same

        close();
        return false;
    } else if (result <= to_receive) {
        data.resize(result);
    } else {
        // This should never happen, we received more bytes than our
        // buffer TODO (mbarczak). Log it and critical exception.

        abort();
        return false;
    }

    return true;
}

bool Connection::isActive() {
    return m_fd > 0;
}

}  // namespace unixsocket
}  // namespace octf
