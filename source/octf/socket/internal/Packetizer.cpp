/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/socket/internal/Packetizer.h>

using namespace std;

namespace octf {

/**
 * Packet layout:
 *
 * +----------------------------------------+
 * | Signature |  Data size  | Packet data  |
 * +----------------------------------------+
 * |          Header         | Actual data  |
 *
 *
 * Data size is the size of data part (excluding header).
 * Data size is encoded in little endian format.
 *
 */

enum packetFormat { signaturePos = 0, dataSizePos = 1, dataPos = 5 };

constexpr int HEADER_SIZE = packetFormat::dataPos;
constexpr unsigned char HEADER_SIGNATURE = 0x7F;

Packetizer::Packetizer(const SocketConnectionShRef &conn)
        : m_conn(conn)
        , m_error(false)
        , m_expectedDataSize(0) {}

Packetizer::~Packetizer() {}

string Packetizer::createHeader(uint32_t dataSize) {
    string header(HEADER_SIZE, '\0');

    // Set packet signature
    header[packetFormat::signaturePos] = HEADER_SIGNATURE;

    // Set data size - little endian format
    // assumes fixed (4B) length of data size field for simplicity
    header[packetFormat::dataSizePos] = dataSize & 0xFF;
    header[packetFormat::dataSizePos + 1] = (dataSize >> 8) & 0xFF;
    header[packetFormat::dataSizePos + 2] = (dataSize >> 16) & 0xFF;
    header[packetFormat::dataSizePos + 3] = (dataSize >> 24) & 0xFF;

    return header;
}

bool Packetizer::sendPacket(const std::string &packetData) {
    string header = createHeader(packetData.size());

    // Send header
    if (!m_conn->send(header)) {
        m_error = true;
        return false;
    }

    // Send actual data
    if (!m_conn->send(packetData)) {
        m_error = true;
        return false;
    }

    return true;
}

bool Packetizer::receivePacket(std::string &data) {
    if (m_error) {
        m_expectedDataSize = 0;
        return false;
    }

    string header;
    unsigned int bytesToReceive = 0;

    // Expecting header
    if (m_conn->receive(header, HEADER_SIZE)) {
        // Process and validate header
        if (!processHeader(header)) {
            m_error = true;
            m_expectedDataSize = 0;
            return false;
        }

    } else {
        m_error = true;
        m_expectedDataSize = 0;
        return false;
    }

    string received;
    unsigned int receivedBytes = 0;

    // Allocate space for data to be received
    data.resize(m_expectedDataSize);

    while (receivedBytes < m_expectedDataSize) {
        // Return false when encountered error or received more than expected
        if (m_error || receivedBytes > m_expectedDataSize) {
            m_expectedDataSize = 0;
            m_error = true;
            return false;
        }

        // Number of bytes that yet to be received
        bytesToReceive = m_expectedDataSize - receivedBytes;

        // Expecting packet's data
        if (!m_conn->receive(received, bytesToReceive)) {
            m_error = true;
            return false;
        }

        // Add received data
        data.replace(receivedBytes, received.size(), received);

        // Update received bytes number
        receivedBytes += received.size();
    }

    return true;
}

bool Packetizer::processHeader(const std::string &header) {
    // Check for signature, raw data is not expected
    if (header[packetFormat::signaturePos] != HEADER_SIGNATURE) {
        return false;
    }

    // Couldn't parse data size
    if (!parseDataSize(header)) {
        return false;
    }

    // Packet size is too large
    if (m_expectedDataSize > ISocketConnection::MAX_PACKET_SIZE) {
        return false;
    }

    return true;
}

bool Packetizer::parseDataSize(const std::string &header) {
    // Check if received entire header
    if (header.size() < HEADER_SIZE) {
        m_error = true;
        m_expectedDataSize = 0;
        return false;
    }

    uint32_t size = 0;

    size += static_cast<uint8_t>(header[packetFormat::dataSizePos]);
    size += static_cast<uint8_t>(header[packetFormat::dataSizePos + 1]) << 8;
    size += static_cast<uint8_t>(header[packetFormat::dataSizePos + 2]) << 16;
    size += static_cast<uint8_t>(header[packetFormat::dataSizePos + 3]) << 24;

    m_expectedDataSize = size;
    return true;
}

bool Packetizer::isError() {
    if (!m_conn->isActive()) {
        m_error = true;
    }

    return m_error;
}

void Packetizer::reset() {
    m_expectedDataSize = 0;
    m_error = 0;
}

}  // namespace octf
