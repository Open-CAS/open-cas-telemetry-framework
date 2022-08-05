/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACECACHELOCAL_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACECACHELOCAL_H

#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/internal/TraceCacheBase.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

class TraceCacheLocal : public TraceCacheBase {
public:
    TraceCacheLocal(const std::string &tracePath);
    virtual ~TraceCacheLocal() = default;

    virtual void clear() override;

    bool read(const google::protobuf::Message &key,
              google::protobuf::Message &value) override;

    bool write(const google::protobuf::Message &key,
               const google::protobuf::Message &value) override;

private:
    proto::TraceCache::Entry *find(proto::TraceCache &cache,
                                   const google::protobuf::Any &key);

    std::string traceCachePath(const std::string &tracePath) const;

    bool isCacheEntryValid(const proto::TraceCache::Entry &entry);

    void eraseCacheEntry(proto::TraceCache &cache,
                         proto::TraceCache::Entry *entry,
                         ProtobufReaderWriter &rw);

private:
    const std::string m_traceCachePath;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACECACHELOCAL_H
