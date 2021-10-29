/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACETRACECREATINGIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACETRACECREATINGIMPL_H

#include <memory>

#include <octf/interface/ITraceExecutor.h>
#include <octf/node/INode.h>
#include <octf/proto/InterfaceTraceCreating.pb.h>

namespace octf {

class TraceManager;

class InterfaceTraceCreatingImpl : public proto::InterfaceTraceCreating {
public:
    /**
     * @brief InterfaceTraceCreatingImpl constructor
     * @param owner Node containing this interface
     * @param traceExecutor trace control object
     */
    InterfaceTraceCreatingImpl(const NodePath &ownerNodePath,
                               ITraceExecutor *traceExecutor);

    virtual ~InterfaceTraceCreatingImpl();

    virtual void StartTracing(::google::protobuf::RpcController *controller,
                              const ::octf::proto::StartTraceRequest *request,
                              ::octf::proto::Void *response,
                              ::google::protobuf::Closure *done) override;

    virtual void StopTracing(::google::protobuf::RpcController *controller,
                             const ::octf::proto::Void *request,
                             ::octf::proto::TraceSummary *response,
                             ::google::protobuf::Closure *done) override;

    virtual void GetTraceSummary(::google::protobuf::RpcController *controller,
                                 const ::octf::proto::Void *request,
                                 ::octf::proto::TraceSummary *response,
                                 ::google::protobuf::Closure *done) override;

    /**
     * @brief Forwards a trace push to the TraceManager
     * @retval 0 - on successful operation.
     * @retval -EINVAL - if queueId couldn't be found
     * @retval -ENOSPC - if ran out of space in internal buffer for trace
     */
    int pushTrace(uint32_t traceQueueId,
                  const void *trace,
                  const uint32_t traceSize);

private:
    void fillTraceSummary(proto::TraceSummary *status);

    bool checkIntegerParameters(const uint32_t value,
                                const std::string &fieldName,
                                const ::google::protobuf::Descriptor *);

    std::unique_ptr<TraceManager> m_traceManager;
    const NodePath m_ownerNodePath;
};
}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACETRACECREATINGIMPL_H
