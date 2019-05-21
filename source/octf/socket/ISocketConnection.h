/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_SOCKET_ISOCKETCONNECTION_H
#define SOURCE_OCTF_SOCKET_ISOCKETCONNECTION_H

#include <memory>
#include <string>

#include <octf/utils/NonCopyable.h>

namespace octf {

class ISocketConnection;
class SocketConfig;

/** Connection Reference */
typedef std::shared_ptr<ISocketConnection> SocketConnectionShRef;

/**
 * @interface ISocketConnection
 *
 * @brief Class which defines interface for socket connection.
 *
 * Caller can send bytes to server or client. Caller can receive bytes from
 * server or client
 *
 */
class ISocketConnection : private NonCopyable {
public:
    ISocketConnection();
    virtual ~ISocketConnection();

    /**
     * @brief Close connection on socket
     */
    virtual void close() = 0;

    /**
     * @brief Send data
     *
     * @param[in] data Data to be sent, number of bytes to be send is equal
     * to string size
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    virtual bool send(const std::string &data) = 0;

    /**
     * @brief Receive data
     *
     * @param[out] data Data buffer containing received bytes
     * @param[in] size - Limit of bytes to receive
     *
     * @note Before receiving procedure all context of data will be cleared
     * @note Call data.size() to get number of received bytes
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    virtual bool receive(std::string &data, size_t size = MAX_PACKET_SIZE) = 0;

    /**
     * @brief Check if connection is active and working
     *
     * @retval true Connection is active and working
     * @retval false Connection inactive, not working
     */
    virtual bool isActive() = 0;

    /**
     * Constant defines maximum number of bytes to receive/send
     * in particular data transfer on connection
     */
    static const size_t MAX_PACKET_SIZE;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_ISOCKETCONNECTION_H
