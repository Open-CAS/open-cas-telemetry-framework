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

constexpr int32_t TRACE_VERSION = 1;

TraceConverter::TraceConverter()
        : m_evDesc(std::make_shared<proto::trace::Event>())
        , m_evIO(std::make_shared<proto::trace::Event>())
        , m_evIOCmpl(std::make_shared<proto::trace::Event>())
        , m_evFsMeta(std::make_shared<proto::trace::Event>())
        , m_evFsFileName(std::make_shared<proto::trace::Event>())
        , m_evFsFileEvent(std::make_shared<proto::trace::Event>()) {}

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
        protoDeviceDesc->set_model(event->device_model);

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
        protoIo->set_writehint(event->write_hint);
        protoIo->set_id(event->id);

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
        protoIoCmpl->set_deviceid(event->dev_id);
        protoIoCmpl->set_refid(event->ref_id);

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
        protoFsMeta->set_fileoffset(event->file_offset);
        protoFsMeta->set_filesize(event->file_size);

        setFileId(protoFsMeta->mutable_fileid(), event->partition_id,
                  &event->file_id);

        return m_evFsMeta;
    }

    case iotrace_event_type_fs_file_name: {
        if (size != sizeof(struct iotrace_event_fs_file_name)) {
            return nullptr;
        }
        auto event =
                static_cast<const struct iotrace_event_fs_file_name *>(trace);

        protobufHdr = m_evFsFileName->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoFsFileName = m_evFsFileName->mutable_filesystemfilename();
        protoFsFileName->set_filename(event->file_name);

        setFileId(protoFsFileName->mutable_fileid(), event->partition_id,
                  &event->file_id);
        setFileId(protoFsFileName->mutable_fileparentid(), event->partition_id,
                  &event->file_parent_id);

        return m_evFsFileName;
    }

    case iotrace_event_type_fs_file_event: {
        if (size != sizeof(struct iotrace_event_fs_file_event)) {
            return nullptr;
        }
        auto event =
                static_cast<const struct iotrace_event_fs_file_event *>(trace);

        protobufHdr = m_evFsFileEvent->mutable_header();
        protobufHdr->set_sid(hdr->sid);
        protobufHdr->set_timestamp(hdr->timestamp);

        auto protoFsFileEvent = m_evFsFileEvent->mutable_filesystemfileevent();
        setFileId(protoFsFileEvent->mutable_fileid(), event->partition_id,
                  &event->file_id);

        switch (event->fs_event_type) {
        case iotrace_fs_event_create:
            protoFsFileEvent->set_fseventtype(trace::FsEventType::Create);
            break;
        case iotrace_fs_event_delete:
            protoFsFileEvent->set_fseventtype(trace::FsEventType::Delete);
            break;
        case iotrace_fs_event_move_to:
            protoFsFileEvent->set_fseventtype(trace::FsEventType::MoveTo);
            break;
        case iotrace_fs_event_move_from:
            protoFsFileEvent->set_fseventtype(trace::FsEventType::MoveFrom);
            break;
        }

        return m_evFsFileEvent;
    }

    default:
        return nullptr;
    }

    return nullptr;
}

void TraceConverter::setFileId(proto::trace::FileId *fileId,
                               uint64_t partition,
                               const iotrace_event_file_id *file_id) {
    fileId->set_id(file_id->id);
    fileId->set_partitionid(partition);

    auto timestamp = fileId->mutable_creationdate();
    timestamp->set_seconds(file_id->ctime.tv_sec);
    timestamp->set_nanos(file_id->ctime.tv_nsec);
}

int32_t TraceConverter::getTraceVersion() const {
    return TRACE_VERSION;
}

}  // namespace octf
