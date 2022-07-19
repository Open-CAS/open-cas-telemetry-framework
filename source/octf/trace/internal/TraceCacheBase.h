/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRACECACHEBASE_H
#define TRACECACHEBASE_H

#include <octf/trace/ITraceCache.h>

namespace octf {

class TraceCacheBase : public ITraceCache {
public:
    TraceCacheBase() = default;
    virtual ~TraceCacheBase() = default;

    virtual bool read(const std::string &key,
                      google::protobuf::Message &value) override;

    virtual bool write(const std::string &key,
                       const google::protobuf::Message &value) override;

    virtual bool read(const std::string &key, uint64_t &value) override;

    virtual bool write(const std::string &key, const uint64_t &value) override;

private:
};

}  // namespace octf

#endif