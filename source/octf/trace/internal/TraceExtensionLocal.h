/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACEEXTENSIONLOCAL_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACEEXTENSIONLOCAL_H

#include <memory>
#include <string>
#include <octf/proto/extensions.pb.h>
#include <octf/trace/ITraceExtension.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

class TraceExtensionLocal : public ITraceExtension {
public:
    TraceExtensionLocal(const std::string &tracePath,
                        const std::string &extName);
    virtual ~TraceExtensionLocal();

    const std::string &getName() const override;

    bool isWritable() const override;

    bool isReady() const override;

    ITraceExtensionWriter &getWriter() override;

    ITraceExtensionReader &getReader() override;

    void remove() override;

private:
    void initTraceExtension(const std::string &tracePath,
                            const std::string &extName);

    bool isStale();

private:
    class Writer;
    class Reader;
    struct Info;
    std::unique_ptr<Info> m_info;
    std::unique_ptr<ITraceExtensionWriter> m_writer;
    std::unique_ptr<ITraceExtensionReader> m_reader;
    std::unique_ptr<ProtobufReaderWriter> m_extrw;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACEEXTENSIONLOCAL_H
