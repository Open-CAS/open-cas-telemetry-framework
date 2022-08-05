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
    rw.lock();

    // Try to read the cache
    if (rw.isFileAvailable() && !rw.read(cache)) {
        // Could not read cache
        return false;
    }

    bool result = false;
    auto entry = find(cache, pKey);

    if (entry) {
        // Entry found for given key

        if (isCacheEntryValid(*entry)) {
            // Entry valid, unpack value
            result = entry->value().UnpackTo(&value);
        }

        if (!result) {
            // Cannot parse the value, just erase it
            eraseCacheEntry(cache, entry, rw);
        }
    }

    return result;
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
    rw.lock();

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

    // Set version
    entry->set_version(getFrameworkConfiguration().getVersion());

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
    rw.lock();

    // Try to read the cache
    if (rw.isFileAvailable()) {
        if (!rw.remove()) {
            throw Exception("Cannot remove trace cache: " + m_traceCachePath);
        }
    }
}

bool TraceCacheLocal::isCacheEntryValid(const proto::TraceCache::Entry &entry) {
    return entry.version() == getFrameworkConfiguration().getVersion();
}

void TraceCacheLocal::eraseCacheEntry(proto::TraceCache &cache,
                                      proto::TraceCache::Entry *entry,
                                      ProtobufReaderWriter &rw) {
    proto::TraceCache newCache;

    for (int i = 0; i < cache.entires_size(); i++) {
        auto iter = cache.mutable_entires(i);

        if (iter != entry) {
            newCache.add_entires()->CopyFrom(*entry);
        }
    }

    rw.write(newCache);
}

}  // namespace octf
