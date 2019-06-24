/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_IOTRACEPLUGIN_H
#define SOURCE_OCTF_TRACE_IOTRACEPLUGIN_H

#include <memory>
#include <octf/interface/ITraceExecutor.h>
#include <octf/plugin/NodePlugin.h>
#include <octf/trace/iotrace_event.h>

namespace octf {

class InterfaceTraceCreatingImpl;

class IOTracePlugin : public NodePlugin, public ITraceExecutor {
public:
    IOTracePlugin(const std::string &pluginId, uint32_t queueCount);
    virtual ~IOTracePlugin();

    bool initCustom() override;

    bool startTrace() override;

    bool stopTrace() override;

    uint32_t getTraceQueueCount() override;

    std::unique_ptr<IRingTraceProducer> createProducer(uint32_t queue) override;

    std::unique_ptr<ITraceConverter> createTraceConverter() override;

    /**
     * @brief Pushes an event to be traced
     *
     * @param ioQueueId
     * @param trace trace event to be stored
     * @param size size of trace event to be stored
     */
    void push(uint32_t ioQueueId, const void *trace, size_t size);

private:
    /**
     * @brief Object implementing trace collecting interface
     */
    std::shared_ptr<InterfaceTraceCreatingImpl> m_tracing;

    /**
     * @brief Number of cache IO queues and the resulting number of spawned
     * trace jobs
     */
    uint32_t m_ioQueueCount;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_IOTRACEPLUGIN_H
