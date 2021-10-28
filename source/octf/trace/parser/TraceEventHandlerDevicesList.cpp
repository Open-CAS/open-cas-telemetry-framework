/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <google/protobuf/util/message_differencer.h>
#include <octf/trace/parser/TraceEventHandlerDevicesList.h>

namespace octf {

TraceEventHandlerDevicesList::TraceEventHandlerDevicesList(
        const std::string &tracePath)
        : TraceEventHandler(tracePath)
        , m_devList() {}

TraceEventHandlerDevicesList::~TraceEventHandlerDevicesList() {}

void TraceEventHandlerDevicesList::handleEvent(
        std::shared_ptr<proto::trace::Event> traceEvent) {
    if (traceEvent->has_devicedescription()) {
        const auto &desc = traceEvent->devicedescription();

        auto iter = m_devList.find(desc.id());
        if (iter != m_devList.end()) {
            if (!google::protobuf::util::MessageDifferencer::Equals(
                        iter->second, desc)) {
                throw Exception("Device description ERROR, ID duplication");
            }
        } else {
            m_devList.emplace(std::make_pair(desc.id(), desc));
        }
    } else {
        // All events of device descriptions have been handled, stop processing
        cancel();
    }
}

void TraceEventHandlerDevicesList::getDevicesList(
        proto::ListDevicesResponse *deviceList) const {
    for (const auto &pair : m_devList) {
        deviceList->add_devices()->CopyFrom(pair.second);
    }
}

}  // namespace octf
