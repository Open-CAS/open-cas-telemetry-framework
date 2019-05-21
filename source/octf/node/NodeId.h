/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_NODEID_H
#define SOURCE_OCTF_NODE_NODEID_H

#include <stdint.h>
#include <string>
#include <vector>

#include <octf/proto/defs.pb.h>

namespace octf {

/**
 * Node ID
 *
 * @note The proper string format of ID is compatible with CLI long key format.
 * For validation cliUtils::isLongKeyValid method can be used.
 *
 * @note In constructor in case of invalid node id, InvalidNodeIdException is
 * thrown.
 */
class NodeId {
public:
    NodeId(const std::string &id);
    NodeId(const proto::NodeId &id);
    NodeId(const NodeId &&id);
    NodeId(const NodeId &id);

    virtual ~NodeId();

    NodeId &operator=(const NodeId &id);
    NodeId &operator=(const NodeId &&id);

    bool operator<(const NodeId &id) const;
    bool operator==(const NodeId &id) const;
    bool operator!=(const NodeId &id) const;

    /**
     * @brief Gets Node ID
     *
     * @return Node ID
     */
    const std::string &getId() const {
        return m_id;
    }

private:
    void validate();

private:
    /**
     * Node ID
     */
    std::string m_id;
};

/**
 * @typedef Node path
 */
typedef std::vector<NodeId> NodePath;

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODEID_H
