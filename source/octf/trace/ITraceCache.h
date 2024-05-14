/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_ITRACECACHE_H
#define SOURCE_OCTF_TRACE_ITRACECACHE_H

#include <google/protobuf/message.h>
#include <octf/utils/NonCopyable.h>

/**
 * @brief This is the interface declaration for a trace cache.
 *
 * In some cases trace postprocessing make take a while. Instead of executing
 * it many times, trace parsers can store value of the trace processing in cache
 * and next time read it from the trace cache.
 */
namespace octf {

class ITraceCache : public NonCopyable {
public:
    ITraceCache() = default;
    virtual ~ITraceCache() = default;

    /**
     * @brief Clears entire content of the cache
     */
    virtual void clear() = 0;

    /**
     * @brief Reads the value from the trace cache for teh specific key
     *
     * @param[in] key Trace cache key
     * @param[out] value Trace cache value for the requested key.
     */
    virtual bool read(const google::protobuf::Message &key,
                      google::protobuf::Message &value) = 0;

    /**
     * @brief Writes the value to the trace cache for teh specific key
     *
     * @param[in] key Trace cache key
     * @param[in] value Trace cache value for the requested key.
     */
    virtual bool write(const google::protobuf::Message &key,
                       const google::protobuf::Message &value) = 0;

    /**
     * @brief Reads the value from the trace cache for teh specific string key
     *
     * @param[in] key Trace cache key
     * @param[out] value Trace cache value for the requested key.
     */
    virtual bool read(const std::string &key,
                      google::protobuf::Message &value) = 0;

    /**
     * @brief Writes the value to the trace cache for teh specific string key
     *
     * @param[in] key Trace cache key
     * @param[in] value Trace cache value for the requested key.
     */
    virtual bool write(const std::string &key,
                       const google::protobuf::Message &value) = 0;

    /**
     * @brief Reads the value from the trace cache for teh specific string key
     *
     * @param[in] key Trace cache key
     * @param[out] value Trace cache value for the requested key.
     */
    virtual bool read(const std::string &key, uint64_t &value) = 0;

    /**
     * @brief Writes the value to the trace cache for teh specific string key
     *
     * @param[in] key Trace cache key
     * @param[in] value Trace cache value for the requested key.
     */
    virtual bool write(const std::string &key, const uint64_t &value) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_ITRACECACHE_H