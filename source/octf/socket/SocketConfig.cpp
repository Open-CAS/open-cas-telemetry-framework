/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/socket/SocketConfig.h>

namespace octf {

SocketConfig::SocketConfig(const std::string &address,
                           SocketImplementation implementation,
                           SocketType type)
        : address(address)
        , implementation(implementation)
        , type(type) {}

SocketConfig::SocketConfig(const SocketConfig &cnfg)
        : address(cnfg.address)
        , implementation(cnfg.implementation)
        , type(cnfg.type) {}

SocketConfig &SocketConfig::operator=(const SocketConfig &cnfg) {
    address = cnfg.address;
    implementation = cnfg.implementation;
    type = cnfg.type;

    return *this;
}

SocketConfig::SocketConfig(const SocketConfig &&cnfg)
        : address(std::move(cnfg.address))
        , implementation(std::move(cnfg.implementation))
        , type(std::move(cnfg.type)) {}

SocketConfig &SocketConfig::operator=(const SocketConfig &&cnfg) {
    address = std::move(cnfg.address);
    implementation = std::move(cnfg.implementation);
    type = std::move(cnfg.type);

    return *this;
}

SocketConfig::~SocketConfig() {}

bool SocketConfig::operator==(const SocketConfig &cnfg) const {
    return this->address == cnfg.address &&
           this->implementation == cnfg.implementation &&
           this->type == cnfg.type;
}

}  // namespace octf
