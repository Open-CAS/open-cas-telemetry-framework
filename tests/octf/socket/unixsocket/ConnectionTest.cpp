/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <octf/node/NodeId.h>
#include <octf/socket/SocketFactory.h>
#include <octf/socket/internal/unixsocket/Client.h>
#include <octf/socket/internal/unixsocket/Server.h>
#include <octf/utils/FrameworkConfiguration.h>

#include <octf/socket/testDefines.h>

using namespace octf;
using namespace std;

static const std::string EXISTENT_PATH =
        octf::getFrameworkConfiguration().getUnixSocketFilePath(
                octf::NodeId("testsocket"));

// Enum is used for flagging what behaviour is expected from server
enum Expect {
    validSocket = 1 << 0,
    clientClose = 1 << 1,
    listenFailure = 1 << 2,
    serverClose = 1 << 3,
    serverSend = 1 << 4
};

// This function is used for starting server in a thread
void startServer(std::string expectedMsg, Expect expect) {
    bool result;
    SocketFactory sFactory;

    SocketServerShRef server =
            sFactory.createServer(EXISTENT_PATH, SocketImplementation::Unix);

    result = server->isActive();

    EXPECT_FALSE(result);

    result = server->open();

    if (expect & Expect::validSocket) {
        EXPECT_TRUE(result);
    }

    SocketConnectionShRef conn = server->listen();

    if (!conn) {
        // Listen is not expected to fail so set failure
        if (!(expect & Expect::listenFailure)) {
            ADD_FAILURE();
        }

        server->close();
        return;
    };

    if (expect & Expect::serverClose) {
        conn->close();
        server->close();

        return;
    }

    string msg;
    result = conn->receive(msg);

    if (!result) {
        // Connection is not expected to close so set failure
        if (!(expect & Expect::clientClose)) {
            ADD_FAILURE();
        }

        server->close();
        return;
    }

    // If client closed connection or listen failed,
    // receive should return false
    if ((expect & Expect::clientClose) | (expect & Expect::listenFailure)) {
        EXPECT_FALSE(result);
    }

    // Check if received message is the same
    if (msg != expectedMsg) {
        ADD_FAILURE();
    }

    conn->close();
    server->close();
    return;
}

// Valid connection
TEST(ConnectionTest, ValidServerReceive) {
    SocketFactory sFactory;

    thread serverThread(startServer, TEST_MESSAGE, Expect::validSocket);
    usleep(100000);

    SocketClientShRef client =
            sFactory.createClient(EXISTENT_PATH, SocketImplementation::Unix);
    SocketConnectionShRef conn = client->connect();

    if (!conn) {
        // Connection failure - don't continue
        serverThread.join();
        FAIL();
    }

    string msg = TEST_MESSAGE;

    if (!conn->send(msg)) {
        // Cannot send data
        ADD_FAILURE();
    }

    serverThread.join();
}

// Connection closed before send from client side
TEST(ConnectionTest, ClientClosedConnection) {
    SocketFactory sFactory;

    thread serverThread(
            startServer, TEST_MESSAGE,
            static_cast<Expect>(Expect::clientClose | Expect::validSocket));
    usleep(100000);

    SocketClientShRef client =
            sFactory.createClient(EXISTENT_PATH, SocketImplementation::Unix);
    SocketConnectionShRef conn = client->connect();

    if (!conn) {
        // Connection failure
        serverThread.join();
        FAIL();
    }

    conn->close();
    serverThread.join();
}

// Connection closed before send  from server side
TEST(ConnectionTest, ServerClosedConnection) {
    bool result;
    SocketFactory sFactory;

    thread serverThread(
            startServer, TEST_MESSAGE,
            static_cast<Expect>(Expect::validSocket | Expect::serverClose));
    usleep(100000);

    SocketClientShRef client =
            sFactory.createClient(EXISTENT_PATH, SocketImplementation::Unix);
    SocketConnectionShRef conn = client->connect();

    if (!conn) {
        // Connection failure
        serverThread.join();
        FAIL();
    }

    string msg = TEST_MESSAGE;

    usleep(100000);

    result = conn->send(msg);

    EXPECT_FALSE(result);

    conn->close();
    serverThread.join();
}
