/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_TRACEEXTENSIONSET_H
#define SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_TRACEEXTENSIONSET_H

#include <set>
#include <string>
#include <octf/trace/ITrace.h>
#include <octf/trace/ITraceExtension.h>

namespace octf {

class TraceExtensionSet : public ITraceExtension,
                          public ITraceExtension::ITraceExtensionReader {
public:
    TraceExtensionSet() = default;
    virtual ~TraceExtensionSet() = default;

    void addTraceExtension(TraceExtensionShRef ext);

    const std::string &getName() const override;

    bool isWritable() const override;

    void remove() override;

    ITraceExtension::ITraceExtensionWriter &getWriter() override;

    ITraceExtension::ITraceExtensionReader &getReader() override;

    void read(uint64_t &sid, google::protobuf::Message &ext) override;

    bool hasNext() override;

    uint64_t getNextSid() override;

private:
    struct TraceExtensionEntry {
        TraceExtensionEntry(TraceExtensionShRef extention);
        bool operator<(const TraceExtensionEntry &other) const;

        TraceExtensionShRef ext;
        uint64_t sid;
    };

private:
    std::set<TraceExtensionEntry> m_set;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_TRACEEXTENSIONSET_H
