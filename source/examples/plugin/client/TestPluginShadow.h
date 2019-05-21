/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_EXAMPLES_PLUGIN_CLIENT_TESTPLUGINSHADOW_H
#define SOURCE_EXAMPLES_PLUGIN_CLIENT_TESTPLUGINSHADOW_H

#include <octf/octf.h>

namespace octf {

/**
 * @brief Test Plugin Shadow
 *
 * An example of plugin shadow
 */
class TestPluginShadow : public NodePluginShadow {
public:
    TestPluginShadow();
    virtual ~TestPluginShadow();

    bool initCustom() override;

    std::shared_ptr<proto::InterfaceCli> getCliInterface() {
        return findInterface<proto::InterfaceCli_Stub>();
    }
};

}  // namespace octf

#endif  // SOURCE_EXAMPLES_PLUGIN_CLIENT_TESTPLUGINSHADOW_H
