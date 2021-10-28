/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>
#include <string>
#include <octf/node/NodeId.h>
#include <octf/socket/ISocketConnection.h>
#include <octf/socket/internal/unixsocket/Server.h>
#include <octf/socket/testDefines.h>
#include <octf/utils/FrameworkConfiguration.h>

using namespace octf;
using namespace std;

static const std::string EXISTENT_PATH =
        octf::getFrameworkConfiguration().getUnixSocketFilePath(
                octf::NodeId("testsocket"));

// Positive test
TEST(ServerTest, PositiveTest) {
    bool result;

    octf::unixsocket::Server server(EXISTENT_PATH);
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();

    EXPECT_TRUE(result);

    result = server.isActive();

    EXPECT_TRUE(result);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);
}

// Socket path is nonexistent
TEST(ServerTest, NonexistentSocketPath) {
    bool result;

    octf::unixsocket::Server server(NONEXISTENT_PATH);
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();

    EXPECT_FALSE(result);

    SocketConnectionShRef conn = server.listen();

    ASSERT_EQ(nullptr, conn);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);
}

// Socket path is empty
TEST(ServerTest, EmptySocketPath) {
    bool result;

    octf::unixsocket::Server server("");
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();

    EXPECT_FALSE(result);

    result = server.isActive();

    EXPECT_FALSE(result);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);
}

// Socket path is too long
TEST(ServerTest, TooLongSocketPath) {
    bool result;

    // Create proper path of maxSocketPathLen + 1 length
    std::string longPath(EXISTENT_PATH);
    int toFill = (MAX_SOCKET_PATH_LEN + 1) - longPath.length();
    std::string socketName(toFill, 'z');
    longPath = longPath + socketName;

    octf::unixsocket::Server server(longPath);
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();

    EXPECT_FALSE(result);

    result = server.isActive();

    EXPECT_FALSE(result);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);
}

// Open socket multiple times
TEST(ServerTest, MultipleOpenSocket) {
    bool result;

    octf::unixsocket::Server server(EXISTENT_PATH);
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();
    EXPECT_TRUE(result);

    result = server.isActive();

    EXPECT_TRUE(result);

    int tries = 10;
    for (int i = 0; i < tries; i++) {
        result = server.open();

        EXPECT_FALSE(result);

        result = server.isActive();

        EXPECT_TRUE(result);
    }

    server.close();
    result = server.isActive();

    EXPECT_FALSE(result);
}

// Close socket multiple times
TEST(ServerTest, MultipleCloseSocket) {
    bool result;

    octf::unixsocket::Server server(EXISTENT_PATH);
    result = server.isActive();

    EXPECT_FALSE(result);

    result = server.open();

    EXPECT_TRUE(result);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);

    int tries = 10;
    for (int i = 0; i < tries; i++) {
        server.close();
        result = server.isActive();

        EXPECT_FALSE(result);
    }
}

// Close socket without open
TEST(ServerTest, CloseWithoutOpen) {
    bool result;

    octf::unixsocket::Server server(EXISTENT_PATH);
    result = server.isActive();

    EXPECT_FALSE(result);

    server.close();

    result = server.isActive();

    EXPECT_FALSE(result);

    int tries = 10;
    for (int i = 0; i < tries; i++) {
        server.close();
        result = server.isActive();

        EXPECT_FALSE(result);
    }
}
