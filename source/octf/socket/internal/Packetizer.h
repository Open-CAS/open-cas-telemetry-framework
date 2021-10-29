/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_SOCKET_INTERNAL_PACKETIZER_H
#define SOURCE_OCTF_SOCKET_INTERNAL_PACKETIZER_H

#include <stdint.h>
#include <list>
#include <octf/socket/ISocketConnection.h>

namespace octf {

/**
 * @brief Class used for sending and receiving entire packets.
 *
 * Caller can be sure that entire data packet
 * has been sent or received.
 *
 * @note This class is private component of socket source tree and intended
 * to be used by common library
 */
class Packetizer {
public:
    /**
     * @brief Creates packetizer class which utilizes given connection
     * @param conn Connection used to send or receive packets
     */
    Packetizer(const SocketConnectionShRef &conn);
    virtual ~Packetizer();

    /**
     * @brief Sends data as a packet
     *
     * @param[in] data Data to be sent
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    bool sendPacket(const std::string &data);

    /**
     * @brief Receives packet
     *
     * @note Blocking function
     *
     * @param[out] Data buffer containing received bytes
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    bool receivePacket(std::string &data);

    /**
     * @return Has any error occurred
     * @retval true Yes
     * @retval false None
     */
    bool isError();

    /**
     * @brief Resets error flags and internal state
     */
    void reset();

private:
    /**
     * @brief Creates header for packet of given data size
     *
     * @param dataSize Size of data in packet
     * @return Header string
     */
    std::string createHeader(uint32_t dataSize);

    /**
     * @brief Reads data size from header. Sets expected packet's data size
     *
     * @param header
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    bool parseDataSize(const std::string &header);

    /**
     * @brief Processes and validates header.
     *
     * @param header
     *
     * @return Result of operation
     * @retval true Success
     * @retval false An error
     */
    bool processHeader(const std::string &header);

private:
    /**
     * Connection used for sending/receiving data
     */
    SocketConnectionShRef m_conn;

    /**
     * Error flag
     */
    bool m_error;

    /**
     * The size of data to be received, known from the header
     */
    size_t m_expectedDataSize;
};

}  // namespace octf

#endif  // SOURCE_OCTF_SOCKET_INTERNAL_PACKETIZER_H
