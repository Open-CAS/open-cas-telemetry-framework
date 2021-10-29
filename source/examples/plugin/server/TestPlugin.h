/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_EXAMPLES_PLUGIN_SERVER_TESTPLUGIN_H
#define SOURCE_EXAMPLES_PLUGIN_SERVER_TESTPLUGIN_H

#include <memory>
#include <octf/octf.h>

namespace octf {

/**
 * @brief Test plugin
 *
 * An example of plugin
 */
class TestPlugin : public NodePlugin {
public:
    TestPlugin();

    virtual ~TestPlugin();

    bool initCustom() override;

    void deinitCustom() override;
};

}  // namespace octf

#endif  // SOURCE_EXAMPLES_PLUGIN_SERVER_TESTPLUGIN_H
