/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERDEVICESLIST_H
#define SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERDEVICESLIST_H

#include <map>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/TraceEventHandler.h>

namespace octf {

/**
 * @brief Handler dedicated to get device list from trace
 */
class TraceEventHandlerDevicesList
        : public octf::TraceEventHandler<proto::trace::Event> {
public:
    /**
     * @see TraceEventHandler
     */
    TraceEventHandlerDevicesList(const std::string &tracePath);

    virtual ~TraceEventHandlerDevicesList();

    virtual void handleEvent(
            std::shared_ptr<proto::trace::Event> traceEvent) override;

    void getDevicesList(proto::ListDevicesResponse *deviceList) const;

    bool compareEvents(const proto::trace::Event *a,
                       const proto::trace::Event *b) override {
        return a->header().sid() < b->header().sid();
    }

private:
    std::map<uint64_t, proto::trace::EventDeviceDescription> m_devList;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_TRACEEVENTHANDLERDEVICESLIST_H
