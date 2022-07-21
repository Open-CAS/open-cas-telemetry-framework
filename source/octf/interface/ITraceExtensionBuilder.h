/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H
#define SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H

#include <google/protobuf/message.h>
#include <cstdint>
#include <octf/interface/ITraceConverter.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/utils/table/Table.h>

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
     * extension and return it in the table
     */
    virtual octf::table::Table buildExtension() = 0;

    virtual const google::protobuf::Message &handleIO(
            const proto::trace::ParsedEvent &io) = 0;

    // Builds empty message of the same type as message in handleIO
    virtual const google::protobuf::Message &GetMessage() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACEXTENSIONBUILDER_H
