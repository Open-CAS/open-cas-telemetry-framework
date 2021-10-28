/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_SOCKETCONFIG_H
#define SOURCE_OCTF_SOCKET_SOCKETCONFIG_H
#include <string>

namespace octf {

/**
 * @brief  According to FHS documentation system programs that maintain
 * transient UNIX-domain sockets must place them in /var/run directory.
 *
 */
const char SOCKET_BASE_PATH[] = "/var/run/octf/sockets/";

/**
 * Supported socket implementation
 */
enum class SocketImplementation {
    /**
     * Unix socket implementation
     */
    Unix,
};

/**
 * Socket type
 */
enum class SocketType {
    /**
     * Server like socket
     */
    Server,

    /**
     * Client like socket
     */
    Client,
};

/**
 * Definition of socket configuration
 */
struct SocketConfig {
    /**
     * @param address Socket address
     * @param implementation Socket implementation
     * @param type Socket type
     */
    SocketConfig(const std::string &address,
                 SocketImplementation implementation,
                 SocketType type);

    SocketConfig(const SocketConfig &cnfg);

    SocketConfig &operator=(const SocketConfig &cnfg);

    SocketConfig(const SocketConfig &&cnfg);

    SocketConfig &operator=(const SocketConfig &&cnfg);

    bool operator==(const SocketConfig &cnfg) const;

    virtual ~SocketConfig();

    /**
     * Socket address
     */
    std::string address;

    /**
     * Socket implementation
     */
    SocketImplementation implementation;

    /**
     * Socket type
     */
    SocketType type;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_SOCKETCONFIG_H
