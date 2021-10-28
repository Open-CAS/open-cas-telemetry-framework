/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/socket/SocketFactory.h>

#include <octf/socket/internal/unixsocket/Client.h>
#include <octf/socket/internal/unixsocket/Server.h>

using namespace std;

namespace octf {

SocketFactory::SocketFactory() {}

SocketFactory::~SocketFactory() {}

SocketServerShRef SocketFactory::createServer(const string &address,
                                              SocketImplementation type) {
    switch (type) {
    case SocketImplementation::Unix:
        return make_shared<unixsocket::Server>(address);
    }

    return NULL;
}

SocketClientShRef SocketFactory::createClient(const string &address,
                                              SocketImplementation socketImpl) {
    switch (socketImpl) {
    case SocketImplementation::Unix:
        return make_shared<unixsocket::Client>(address);
    }

    return NULL;
}

}  // namespace octf
