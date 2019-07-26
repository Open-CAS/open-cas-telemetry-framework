/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/trace/IOTracePlugin.h>

#include <octf/interface/InterfaceCliImpl.h>
#include <octf/interface/InterfaceTraceCreatingImpl.h>
#include <octf/interface/TraceConverter.h>
#include <octf/interface/TraceProducerLocal.h>

namespace octf {

IOTracePlugin::IOTracePlugin(const std::string &pluginId, uint32_t queueCount)
        : NodePlugin(NodeId(pluginId))
        , ITraceExecutor()
        , m_tracing(nullptr)
        , m_ioQueueCount(queueCount) {}

IOTracePlugin::~IOTracePlugin() {}

bool IOTracePlugin::initCustom() {
    m_tracing =
            createInterface<InterfaceTraceCreatingImpl>(getNodePath(), this);
    if (!m_tracing) {
        throw Exception("Error creating CAS trace plugin interface");
    }
    if (!createInterface<InterfaceCliImpl>(this)) {
        throw Exception("Error creating plugin CLI interface");
    }
    return true;
}

bool IOTracePlugin::startTrace() {
    return true;
}

bool IOTracePlugin::stopTrace() {
    return true;
}

uint32_t IOTracePlugin::getTraceQueueCount() {
    return m_ioQueueCount;
}

std::unique_ptr<IRingTraceProducer> IOTracePlugin::createProducer(
        uint32_t queue) {
    return std::unique_ptr<TraceProducerLocal>(new TraceProducerLocal(queue));
}

std::unique_ptr<ITraceConverter> IOTracePlugin::createTraceConverter() {
    return std::unique_ptr<TraceConverter>(new TraceConverter());
}

void octf::IOTracePlugin::push(uint32_t ioQueueId,
                               const void *trace,
                               size_t size) {
    m_tracing->pushTrace(ioQueueId, trace, size);
}

}  // namespace octf
