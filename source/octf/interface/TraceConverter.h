/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_TRACECONVERTER_H
#define SOURCE_OCTF_INTERFACE_TRACECONVERTER_H

#include <sys/types.h>
#include <string>
#include <octf/interface/ITraceConverter.h>
#include <octf/proto/trace.pb.h>

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

private:
    std::shared_ptr<proto::trace::Event> m_protobufEvent;
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_TRACECONVERTER_H
