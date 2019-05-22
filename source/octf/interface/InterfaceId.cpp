/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceId.h>
#include <octf/proto/opts.pb.h>

namespace octf {

InterfaceId::InterfaceId()
        : m_name("")
        , m_version(1) {}

InterfaceId::InterfaceId(const std::string &name,
                         const InterfaceVersion_t version)
        : m_name(name)
        , m_version(version) {}

InterfaceId::InterfaceId(const proto::InterfaceId &id)
        : m_name(id.name())
        , m_version(id.version()) {}

bool InterfaceId::operator==(const InterfaceId &id) const {
    return ((this->m_name.compare(id.getName()) == 0) &&
            (this->m_version == id.getVersion()));
}

InterfaceId::InterfaceId(const google::protobuf::ServiceDescriptor *desc)
        : m_name(desc->name())
        , m_version() {
    m_version = desc->options().GetExtension(proto::opts_interface).version();
}

}  // namespace octf
