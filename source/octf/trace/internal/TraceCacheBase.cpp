/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/internal/TraceCacheBase.h>

namespace octf {

bool TraceCacheBase::read(const std::string &key,
                          google::protobuf::Message &value) {
    proto::TraceCache::SimpleKey simpleKey;
    ITraceCache *cache = this;

    simpleKey.set_name(key);
    return cache->read(simpleKey, value);
};

bool TraceCacheBase::write(const std::string &key,
                           const google::protobuf::Message &value) {
    proto::TraceCache::SimpleKey simpleKey;
    ITraceCache *cache = this;

    simpleKey.set_name(key);
    return cache->write(simpleKey, value);
};

bool TraceCacheBase::read(const std::string &key, uint64_t &value) {
    proto::TraceCache::SimpleKey simpleKey;
    proto::TraceCache::SimpleValue simpleValue;
    ITraceCache *cache = this;

    simpleKey.set_name(key);

    if (!cache->read(simpleKey, simpleValue)) {
        return false;
    }

    value = simpleValue.value();
    return true;
};

bool TraceCacheBase::write(const std::string &key, const uint64_t &value) {
    proto::TraceCache::SimpleKey simpleKey;
    proto::TraceCache::SimpleValue simpleValue;
    ITraceCache *cache = this;

    simpleKey.set_name(key);
    simpleValue.set_value(value);

    return cache->write(simpleKey, simpleValue);
}

}  // namespace octf