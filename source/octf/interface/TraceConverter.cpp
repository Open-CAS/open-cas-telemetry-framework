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
        : m_evDesc(std::make_shared<proto::trace::Event>())
        , m_evIO(std::make_shared<proto::trace::Event>())
        , m_evIOCmpl(std::make_shared<proto::trace::Event>())
        , m_evFsMeta(std::make_shared<proto::trace::Event>()) {}

std::shared_ptr<const google::protobuf::Message> TraceConverter::convertTrace(
        const void *trace,
        uint32_t size) {
    using namespace proto;

    if (size < sizeof(struct iotrace_event_hdr)) {
        return nullptr;
    }

    auto hdr = static_cast<const struct iotrace_event_hdr *>(trace);

    // NOTE: Because of performance reason, for each event variant we keep
    // separate message. Thus we avoid data reallocation when switching between
    // event variants.

    trace::EventHeader *protobufHdr;

    switch (hdr->type) {
    case iotrace_event_type_device_desc: {
        if (size != sizeof(struct iotrace_event_device_desc)) {
            return nullptr;
        }
        auto event =
                static_cast<const struct iotrace_event_device_desc *>(trace);

        protobufHdr = m_evDesc->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoDeviceDesc = m_evDesc->mutable_devicedescription();
        protoDeviceDesc->set_id(event->id);
        protoDeviceDesc->set_name(event->device_name);
        protoDeviceDesc->set_size(event->device_size);

        return m_evDesc;
    }

    case iotrace_event_type_io: {
        if (size != sizeof(struct iotrace_event)) {
            return nullptr;
        }

        auto event = static_cast<const struct iotrace_event *>(trace);

        protobufHdr = m_evIO->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoIo = m_evIO->mutable_io();
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

        return m_evIO;
    }

    case iotrace_event_type_io_cmpl: {
        if (size != sizeof(struct iotrace_event_completion)) {
            return nullptr;
        }

        auto event =
                static_cast<const struct iotrace_event_completion *>(trace);

        protobufHdr = m_evIOCmpl->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoIoCmpl = m_evIOCmpl->mutable_iocompletion();

        protoIoCmpl->set_lba(event->lba);
        protoIoCmpl->set_len(event->len);
        protoIoCmpl->set_error(event->error);

        return m_evIOCmpl;
    }

    case iotrace_event_type_fs_meta: {
        if (size != sizeof(struct iotrace_event_fs_meta)) {
            return nullptr;
        }
        auto event = static_cast<const struct iotrace_event_fs_meta *>(trace);

        protobufHdr = m_evFsMeta->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoFsMeta = m_evFsMeta->mutable_filesystemmeta();
        protoFsMeta->set_refsid(event->ref_sid);
        protoFsMeta->set_fileid(event->file_id);
        protoFsMeta->set_fileoffset(event->file_offset);
        protoFsMeta->set_filesize(event->file_size);

        return m_evFsMeta;
    }

    default:
        return nullptr;
    }

    return nullptr;
}

}  // namespace octf
