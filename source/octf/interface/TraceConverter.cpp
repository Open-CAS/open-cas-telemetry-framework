/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <sys/types.h>
#include <string>
#include <octf/interface/TraceConverter.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/iotrace_event.h>

namespace octf {

TraceConverter::TraceConverter()
        : m_protobufEvent(std::make_shared<proto::trace::Event>()) {}

std::shared_ptr<const google::protobuf::Message> TraceConverter::convertTrace(
        const void *trace,
        uint32_t size) {
    using namespace proto;

    if (size < sizeof(struct iotrace_event_hdr)) {
        return nullptr;
    }

    m_protobufEvent->Clear();
    auto hdr = static_cast<const struct iotrace_event_hdr *>(trace);
    trace::EventHeader *protobufHdr;

    switch (hdr->type) {
    case iotrace_event_type_device_desc: {
        if (size != sizeof(struct iotrace_event_device_desc)) {
            return nullptr;
        }
        auto event =
                static_cast<const struct iotrace_event_device_desc *>(trace);
        auto protoDeviceDesc = m_protobufEvent->mutable_devicedescription();
        protobufHdr = m_protobufEvent->mutable_header();
        protoDeviceDesc->set_id(event->id);
        protoDeviceDesc->set_name(event->device_name);
        protoDeviceDesc->set_size(event->device_size);
        break;
    }

    case iotrace_event_type_io: {
        if (size != sizeof(struct iotrace_event)) {
            return nullptr;
        }
        auto event = static_cast<const struct iotrace_event *>(trace);
        auto protoIo = m_protobufEvent->mutable_io();
        protobufHdr = m_protobufEvent->mutable_header();
        switch (event->operation) {
        case iotrace_event_operation_rd:
            protoIo->set_operation(trace::IoType::Read);
            break;
        case iotrace_event_operation_wr:
            protoIo->set_operation(trace::IoType::Write);
            break;
        case iotrace_event_operation_discard:
            protoIo->set_operation(trace::IoType::Discard);
            break;
        default:
            return nullptr;
        }
        protoIo->set_flush(!!(event->flags & iotrace_event_flag_flush));
        protoIo->set_fua(!!(event->flags & iotrace_event_flag_fua));
        protoIo->set_lba(event->lba);
        protoIo->set_len(event->len);
        protoIo->set_ioclass(event->io_class);
        protoIo->set_deviceid(event->dev_id);
        break;
    }

    default:
        return nullptr;
    }

    protobufHdr->set_sid(hdr->sid);
    protobufHdr->set_timestamp(hdr->timestamp);
    return m_protobufEvent;
}

}  // namespace octf
