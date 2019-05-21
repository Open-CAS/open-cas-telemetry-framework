/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/ProtoConverter.h>

namespace octf {
namespace protoconverter {
void convertNodeId(proto::NodeId *nodeProto, const NodeId &node) {
    nodeProto->set_id(node.getId());
}

void convertInterfaceId(proto::InterfaceId *interfaceProto,
                        const InterfaceId &interface) {
    interfaceProto->set_name(interface.getName());
    interfaceProto->set_version(interface.getVersion());
}

void convertNodePath(proto::NodePath *nodePathProto, const NodePath &nodePath) {
    for (const auto &nodeId : nodePath) {
        convertNodeId(nodePathProto->add_node(), nodeId);
    }
}

int encodeVarint32(uint8_t *buffer, uint64_t size, int value) {
    unsigned int uValue = static_cast<unsigned int>(value);

    // Handle small values separately for performance
    if (uValue < 0x80) {
        if (size) {
            *buffer = uValue;
            size = 1;
            return 1;
        } else {
            return 0;
        }
    }

    uint32_t varIntSize = 0;

    do {
        if (0 == size) {
            return 0;
        }

        uint8_t byte = uValue & 0x7F;
        *buffer = byte;
        uValue >>= 7;

        if (uValue) {
            *buffer |= 0x80;
        }

        varIntSize++;
        buffer++;
        size--;

    } while (uValue);

    return varIntSize;
}

int decodeVarint32(const uint8_t *buffer, uint64_t size, int &value) {
    unsigned int uValue = 0;

    if (0 == size) {
        return 0;
    }

    // Handle small values separately for performance
    if (*buffer < 0x80) {
        value = *buffer;
        return 1;
    }

    for (int i = 0; i < MAX_VARINT32_BYTES; i++) {
        if (0 == size) {
            return 0;
        }

        uint8_t byte = *buffer;
        int byteNoMSB = byte & 0x7F;

        uValue |= byteNoMSB << (i * 7);

        if (byte < 0x80) {
            value = static_cast<int>(uValue);
            return i + 1;
        }

        buffer++;
        size--;
    }

    return 0;
}

}  // namespace protoconverter
}  // namespace octf
