/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_MODULESDISCOVER_H
#define SOURCE_OCTF_UTILS_MODULESDISCOVER_H

#include <octf/node/INode.h>

namespace octf {

/**
 * @class Utilities class which can be use in order to discover available
 * modules
 */
class ModulesDiscover {
public:
    ModulesDiscover() = default;
    virtual ~ModulesDiscover() = default;

    /**
     * @brief Discovers available
     *
     * @param[out] modules List of node IDs of available modules (plugins and/or
     * service)
     */
    void getModulesList(NodesIdList &modules);
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_MODULESDISCOVER_H
