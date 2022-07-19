/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/internal/TraceCacheLocal.h>

#include <google/protobuf/util/message_differencer.h>
#include <octf/proto/traceDefinitions.pb.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

static constexpr char CACHE_FILE_NAME[] = "octf.cache";

TraceCacheLocal::TraceCacheLocal(const std::string &tracePath)
        : TraceCacheBase()
        , m_traceCachePath(traceCachePath(tracePath)) {}

bool TraceCacheLocal::read(const google::protobuf::Message &key,
                           google::protobuf::Message &value) {
    proto::TraceCache cache;
    google::protobuf::Any pKey;

    if (!pKey.PackFrom(key)) {
        // Cannot pack key
        return false;
    }

    ProtobufReaderWriter rw(m_traceCachePath);

    // Try to read the cache
    if (rw.isFileAvailable() && !rw.read(cache)) {
        // Could not read cache
        return false;
    }

    auto entry = find(cache, pKey);
    if (entry) {
        // Entry found for given key, unpack value
        return entry->value().UnpackTo(&value);
    }

    return false;
}

bool TraceCacheLocal::write(const google::protobuf::Message &key,
                            const google::protobuf::Message &value) {
    proto::TraceCache cache;
    google::protobuf::Any pKey;

    if (!pKey.PackFrom(key)) {
        // Cannot pack key
        return false;
    }

    ProtobufReaderWriter rw(m_traceCachePath);

    // Try to read the cache
    if (rw.isFileAvailable() && !rw.read(cache)) {
        // Could not read cache
        return false;
    }

    auto entry = find(cache, pKey);
    if (!entry) {
        // Entry not found, create new one
        entry = cache.add_entires();
        entry->mutable_key()->CopyFrom(pKey);
    }

    // Update value
    if (!entry->mutable_value()->PackFrom(value)) {
        return false;
    }

    return rw.write(cache);
}

proto::TraceCache::Entry *TraceCacheLocal::find(
        proto::TraceCache &cache,
        const google::protobuf::Any &key) {
    for (int i = 0; i < cache.entires_size(); i++) {
        auto entry = cache.mutable_entires(i);

        if (google::protobuf::util::MessageDifferencer::Equals(entry->key(),
                                                               key)) {
            return entry;
        }
    }

    return nullptr;
}

std::string TraceCacheLocal::traceCachePath(
        const std::string &tracePath) const {
    return getFrameworkConfiguration().getTraceDir() + "/" + tracePath + "/" +
           CACHE_FILE_NAME;
}

void TraceCacheLocal::clear() {
    ProtobufReaderWriter rw(m_traceCachePath);

    // Try to read the cache
    if (rw.isFileAvailable()) {
        if (!rw.remove()) {
            throw Exception("Cannot remove trace cache: " + m_traceCachePath);
        }
    }
}

}  // namespace octf
