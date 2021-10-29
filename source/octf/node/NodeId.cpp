/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/node/NodeId.h>

#include <octf/cli/internal/OptionsValidation.h>
#include <octf/utils/Exception.h>

namespace octf {

NodeId::NodeId(const std::string &id)
        : m_id(id) {
    validate();
}

NodeId::NodeId(const proto::NodeId &id)
        : m_id(id.id()) {
    validate();
}

NodeId::NodeId(const NodeId &&id)
        : m_id(std::move(id.m_id)) {
    validate();
}

NodeId::NodeId(const NodeId &id)
        : m_id(id.m_id) {
    validate();
}

NodeId::~NodeId() {}

NodeId &NodeId::operator=(const NodeId &id) {
    m_id = id.m_id;

    return *this;
}

NodeId &NodeId::operator=(const NodeId &&id) {
    m_id = std::move(id.m_id);
    return *this;
}

bool NodeId::operator<(const NodeId &id) const {
    return this->m_id < id.m_id;
}

bool NodeId::operator==(const NodeId &id) const {
    return m_id == id.m_id;
}

bool NodeId::operator!=(const NodeId &id) const {
    return m_id != id.m_id;
}

void NodeId::validate() {
    if (!cli::utils::isLongKeyValid(m_id)) {
        throw Exception("Invalid id of Node ID ");
    }
}

}  // namespace octf
