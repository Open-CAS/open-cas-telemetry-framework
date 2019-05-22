/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_IINTERFACE_H
#define SOURCE_OCTF_INTERFACE_IINTERFACE_H

#include <octf/interface/InterfaceId.h>

namespace octf {

class IInterface {
public:
    virtual ~IInterface() = default;
    virtual InterfaceId getInterfaceId() const = 0;
};
}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_IINTERFACE_H
