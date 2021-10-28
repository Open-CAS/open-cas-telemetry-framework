/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>
#include <vector>
#include <octf/node/NodeId.h>
#include <octf/socket/SocketFactory.h>
#include <octf/socket/internal/unixsocket/Client.h>
#include <octf/socket/internal/unixsocket/Server.h>
#include <octf/socket/testDefines.h>
#include <octf/utils/FrameworkConfiguration.h>

using namespace octf;
using namespace std;

static const std::string EXISTENT_PATH =
        octf::getFrameworkConfiguration().getUnixSocketFilePath(
                octf::NodeId("testsocket"));

// Attempt to connect without running server
TEST(ClientTest, NoServerConnection) {
    unixsocket::Client client(EXISTENT_PATH);
    SocketConnectionShRef conn = client.connect();

    ASSERT_EQ(nullptr, conn);
}

// Empty socket path
TEST(ClientTest, EmptySocketPath) {
    unixsocket::Client client("");
    SocketConnectionShRef conn = client.connect();

    ASSERT_EQ(nullptr, conn);
}

// Multiple instances of clients - same socket
// First they are created then they try to connect
TEST(ClientTest, MultipleClientsSameSocket) {
    vector<SocketClientShRef> clients;
    SocketConnectionShRef conn;
    SocketFactory sFactory;

    int instances = 10;
    for (int i = 0; i < instances; i++) {
        clients.push_back(sFactory.createClient(EXISTENT_PATH,
                                                SocketImplementation::Unix));
    }

    for (int i = 0; i < instances; i++) {
        conn = clients[i]->connect();

        ASSERT_EQ(nullptr, conn);
    }

    for (int i = 0; i < instances; i++) {
        clients[i].reset();
    }

    clients.clear();
}

// Use too long socket path
TEST(ClientTest, TooLongSocketPath) {
    // Create proper path of maxSocketPathLen + 1 length
    std::string longPath("/var/lib/octf");
    int toFill = (MAX_SOCKET_PATH_LEN + 1) - longPath.length();
    std::string socketName(toFill, 'z');
    longPath = longPath + socketName;

    unixsocket::Client client(longPath);
    SocketConnectionShRef conn = client.connect();

    ASSERT_EQ(nullptr, conn);
}

// Multiple connect calls
TEST(ClientTest, MultipleConnect) {
    unixsocket::Client client(EXISTENT_PATH);
    SocketConnectionShRef conn;

    int tries = 10;
    for (int i = 0; i < tries; i++) {
        conn = client.connect();

        ASSERT_EQ(nullptr, conn);
    }
}
