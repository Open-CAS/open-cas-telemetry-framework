/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/socket/ISocketConnection.h>

namespace octf {

const size_t ISocketConnection::MAX_PACKET_SIZE = 32ULL * 1024ULL * 1024ULL;

ISocketConnection::ISocketConnection() {}

ISocketConnection::~ISocketConnection() {}

}  // namespace octf
