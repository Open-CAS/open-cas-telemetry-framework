/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRACECACHELOCAL_H
#define TRACECACHELOCAL_H

#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/internal/TraceCacheBase.h>

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

private:
    const std::string m_traceCachePath;
};

}  // namespace octf

#endif  // TRACECACHELOCAL_H