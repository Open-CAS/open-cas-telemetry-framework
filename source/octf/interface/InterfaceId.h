/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACEID_H
#define SOURCE_OCTF_INTERFACE_INTERFACEID_H

#include <google/protobuf/descriptor.h>
#include <stdint.h>
#include <string>

#include <octf/proto/defs.pb.h>

/**
 * @typedef Interface version
 */
typedef uint32_t InterfaceVersion_t;

namespace octf {

class InterfaceId {
public:
    InterfaceId();

    InterfaceId(const google::protobuf::ServiceDescriptor *desc);

    InterfaceId(const proto::InterfaceId &id);

    InterfaceId(const std::string &name, const InterfaceVersion_t version);

    virtual ~InterfaceId() = default;

    bool operator==(const InterfaceId &id) const;

    const std::string &getName() const {
        return m_name;
    }

    InterfaceVersion_t getVersion() const {
        return m_version;
    }

private:
    std::string m_name;
    InterfaceVersion_t m_version;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACEID_H
