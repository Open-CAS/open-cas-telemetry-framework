/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_ITRACEEXTENSIONBUILDER_H
#define SOURCE_OCTF_INTERFACE_ITRACEEXTENSIONBUILDER_H

#include <google/protobuf/message.h>
#include <functional>
#include <list>
#include <octf/interface/ITraceConverter.h>
#include <octf/proto/parsedTrace.pb.h>
#include <octf/utils/Types.h>
#include <octf/utils/table/Table.h>

namespace octf {

/**
 * @brief The interface which should be able to build an trace extensions for
 * the specific trace.
 *
 * @tparam TraceEvent The message prototype of trace for which the extension is
 * produced
 */
template <typename TraceEvent>
class ITraceExtensionBuilder {
public:
    virtual ~ITraceExtensionBuilder() = default;

    /**
     * @brief Gets the name of trace extension
     *
     * @return The name of trace extension
     */
    virtual const std::string &getName() const = 0;

    /**
     * @brief Gets the prototype of message of the trace extension
     */
    virtual MessageShRef getExtensionMessagePrototype() = 0;

    /**
     * @brief The trace extension building procedure step to handle trace event
     *
     * The extension builder might required a procedure which parses the trace a
     * couple of times. This is handler for the step to handle the trace.
     *
     * @retval true Continue trace parsing
     * @retval false Stop trace parsing, the step finished its job
     */
    typedef std::function<bool(const TraceEvent &event)> BuildStepEventHandler;

    /**
     * @brief Gets the build process description defined by the list of steps
     * to execute.
     *
     * @return The List of steps for building extensions
     */
    virtual const std::list<BuildStepEventHandler> &getBuildProcess() = 0;

    /**
     * @brief Checks it the trace extension is ready for given trace
     *
     * @return true The trace extension is ready and can be saved respectively
     * @return false The trace extension not ready, further processing needed
     */
    virtual bool isTraceExtensionReady() = 0;

    /**
     * @brief Gets the trace extension for the specific trace event
     *
     * @return The trace extension
     */
    virtual const google::protobuf::Message &getTraceExtension() = 0;
};

}  //  namespace octf

#endif  // SOURCE_OCTF_INTERFACE_ITRACEEXTENSIONBUILDER_H
