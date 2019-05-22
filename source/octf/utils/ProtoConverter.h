/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_PROTOCONVERTER_H
#define SOURCE_OCTF_UTILS_PROTOCONVERTER_H

#include <octf/interface/InterfaceId.h>
#include <octf/node/NodeId.h>

namespace octf {
namespace protoconverter {
/**
 * @brief Protocol buffer to classes conversions
 */

/**
 * @brief Convert NodeId info proto::NodeId class
 *
 * @param[out] nodeProto Protocol buffer object of NodeId
 * @param node Node identifier
 */
void convertNodeId(proto::NodeId *nodeProto, const NodeId &node);

/**
 * @brief Convert InterfaceId into proto::InterfaceId class
 *
 * @param interfaceProto[out] Protocol buffer object of InterfaceId
 * @param interface Interface identifier
 */
void convertInterfaceId(proto::InterfaceId *interfaceProto,
                        const InterfaceId &interface);

/**
 * @brief Convert NodePath info proto::NodePath class
 *
 * @param nodePathProto[out] Protocol buffer object of NodePath
 * @param nodePath Path of node, consisting of Node identifiers of
 * the node and all it's predecessors
 */
void convertNodePath(proto::NodePath *nodePathProto, const NodePath &nodePath);

static constexpr int MAX_VARINT32_BYTES = 5;

/**
 * @brief Write given value to byte buffer in varint format.
 *
 * For more details about varint encoding visit the following link:
 * @link https://developers.google.com/protocol-buffers/docs/encoding
 *
 * @param buffer Buffer pointer
 * @param size Size of buffer
 * @param value Value to be written to buffer
 * @return How many bytes were written to buffer (after encoding)
 */
int encodeVarint32(uint8_t *buffer, uint64_t size, int value);

/**
 * @brief Read varint value from byte buffer:
 *
 * For more details about varint encoding visit the following link:
 * @link https://developers.google.com/protocol-buffers/docs/encoding
 *
 * @param buffer Buffer pointer
 * @param size Size of buffer
 * @param[out] value Value read from buffer
 * @return How many bytes were read from the buffer
 */
int decodeVarint32(const uint8_t *buffer, uint64_t size, int &value);

}  // namespace protoconverter
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_PROTOCONVERTER_H
