/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACETRACEMANAGEMENTIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACETRACEMANAGEMENTIMPL_H
#include <octf/node/INode.h>
#include <octf/proto/InterfaceTraceManagement.pb.h>

namespace octf {

class InterfaceTraceManagementImpl : public proto::InterfaceTraceManagement {
public:
    /**
     * @brief Create trace management interface
     * @param TracePrefix - Usually a plugin should not have access to other
     * plugins' traces. TracePrefix has to be present at the beginning of a
     * trace path to be seen by this plugin. To be able to list all traces,
     * pass an empty string.
     */
    InterfaceTraceManagementImpl(std::string tracePrefix);
    virtual ~InterfaceTraceManagementImpl();

    virtual void listTraces(::google::protobuf::RpcController *controller,
                            const ::octf::proto::TracePathPrefix *request,
                            ::octf::proto::TraceList *response,
                            ::google::protobuf::Closure *done) override;

    virtual void removeTraces(::google::protobuf::RpcController *controller,
                              const ::octf::proto::RemoveTracesRequest *request,
                              ::octf::proto::TraceList *response,
                              ::google::protobuf::Closure *done) override;

    virtual void getTraceSummary(::google::protobuf::RpcController *controller,
                                 const ::octf::proto::TracePath *request,
                                 ::octf::proto::TraceSummary *response,
                                 ::google::protobuf::Closure *done) override;

    virtual void clearTraceCache(::google::protobuf::RpcController *controller,
                                 const ::octf::proto::TracePathPrefix *request,
                                 ::octf::proto::TraceList *response,
                                 ::google::protobuf::Closure *done) override;

private:
    std::string m_tracePrefix;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACETRACEMANAGEMENTIMPL_H
