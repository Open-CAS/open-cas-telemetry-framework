/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H
#define SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H

#include <cstdint>
#include <octf/interface/ITraceConverter.h>
#include <octf/proto/parsedTrace.pb.h>

namespace octf {

/**
 * @brief The implementation of this interface should be able
 * to build extensions based on OCTF data
 */
class ITraceExtensionBuilder {
public:
    virtual ~ITraceExtensionBuilder() = default;
    /**
     * @brief A specific implementation of the builder will build
     * extension and store it in the local private field
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual void buildExtension(const proto::trace::ParsedEvent &io) = 0;
    /**
     * @brief A specific implementation of the executor will
     * serialize extension saved in local private field of implementing
     * class to binary OCTF format
     * @retval True - on successful operation.
     * @retval False - if any error occurred.
     */
    virtual void serializeExtension(const proto::trace::ParsedEvent &io) = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H
