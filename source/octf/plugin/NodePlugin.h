/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_PLUGIN_NODEPLUGIN_H
#define SOURCE_OCTF_PLUGIN_NODEPLUGIN_H

#include <memory>
#include <octf/node/INode.h>
#include <octf/node/NodeServer.h>

namespace octf {

class ServiceShadow;

class NodePlugin : public NodeServer {
public:
    NodePlugin(const NodeId &id);
    virtual ~NodePlugin();

    // Method finalized, to extend initialization override initCustom
    bool initCommon() final;

    // Method finalized, to extend deinitialization override deinitCustom
    void deinitCommon() final;

private:
    std::shared_ptr<ServiceShadow> m_nodeService;
};

}  // namespace octf

#endif  // SOURCE_OCTF_PLUGIN_NODEPLUGIN_H
