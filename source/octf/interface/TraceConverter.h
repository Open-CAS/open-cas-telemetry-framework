/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_TRACECONVERTER_H
#define SOURCE_OCTF_INTERFACE_TRACECONVERTER_H

#include <sys/types.h>
#include <string>
#include <octf/interface/ITraceConverter.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/iotrace_event.h>

namespace octf {

/**
 * @brief This converter converts internal trace event structure
 * to Google Protobuf Message format.
 */
class TraceConverter : public ITraceConverter {
public:
    TraceConverter();
    virtual ~TraceConverter() = default;

    std::shared_ptr<const google::protobuf::Message> convertTrace(
            const void *trace,
            uint32_t size) override;

    int32_t getTraceVersion() override;

private:
    void setFileId(proto::trace::FileId *fileId,
                   uint64_t partition,
                   const iotrace_event_file_id *file_id);

    std::shared_ptr<proto::trace::Event> m_evDesc;
    std::shared_ptr<proto::trace::Event> m_evIO;
    std::shared_ptr<proto::trace::Event> m_evIOCmpl;
    std::shared_ptr<proto::trace::Event> m_evFsMeta;
    std::shared_ptr<proto::trace::Event> m_evFsFileName;
    std::shared_ptr<proto::trace::Event> m_evFsFileEvent;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_TRACECONVERTER_H
