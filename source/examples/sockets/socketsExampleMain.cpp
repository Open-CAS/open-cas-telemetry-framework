/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/socket/SocketFactory.h>
#include <octf/utils/Log.h>

using namespace octf;

void help(const char *program) {
    printf("usage: %s {server|client}\n", program);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Invalid input arguments\n");
        help(argv[0]);
        exit(1);
    }

    if (std::string(argv[1]) == "server") {
        SocketServerShRef srv = SocketFactory::createServer(
                "octf.service", SocketImplementation::Unix);

        if (!srv->open()) {
            /* Cannot open server socket */
            return -1;
        }

        SocketConnectionShRef conn = srv->listen();
        if (!conn) {
            /* No connection, no client, return */
            return -1;
        }

        std::string msg;
        if (!conn->receive(msg)) {
            /* Cannot receive data */
            return -1;
        }

        log::cout << "Client request is: " << msg << std::endl;
        msg = "I'm here, welcome";

        conn->send(msg);
    } else if (std::string(argv[1]) == "client") {
        SocketClientShRef clnt = SocketFactory::createClient(
                "octf.service", SocketImplementation::Unix);

        SocketConnectionShRef conn = clnt->connect();
        if (!conn) {
            /* Connection failure */
            return -1;
        }

        std::string msg = "This is Client message. Hello!";
        if (!conn->send(msg)) {
            /* Cannot send data */
            return -1;
        }

        if (!conn->receive(msg)) {
            return -1;
        }

        log::cout << "Server response is: " << msg << std::endl;
    } else {
        log::cerr << "Invalid input arguments\n";
        help(argv[0]);
        exit(1);
    }

    return 0;
}
