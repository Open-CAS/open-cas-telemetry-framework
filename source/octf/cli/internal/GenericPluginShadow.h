/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_GENERICPLUGINSHADOW_H
#define SOURCE_OCTF_CLI_GENERICPLUGINSHADOW_H

#include <octf/plugin/NodePluginShadow.h>
#include <octf/proto/InterfaceCLI.pb.h>

namespace octf {

/**
 * @brief Generic Plugin shadow for execution auto CLI commands
 */
class GenericPluginShadow : public NodePluginShadow {
public:
    GenericPluginShadow(const std::string &name);
    virtual ~GenericPluginShadow();

    bool initCustom() override;

    void deinitCustom() override;

    std::shared_ptr<proto::InterfaceCli> getCliInterface() {
        return findInterface<proto::InterfaceCli_Stub>();
    }
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_GENERICPLUGINSHADOW_H
