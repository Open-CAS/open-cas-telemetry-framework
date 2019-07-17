/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceTraceParsingImpl.h>

#include <octf/trace/parser/IoTraceEventHandler.h>
#include <octf/trace/parser/IoTraceEventHandlerCsvPriner.h>
#include <octf/trace/parser/IoTraceEventHandlerJsonPrinter.h>
#include <octf/utils/Exception.h>

namespace octf {

class IoTraceEventHandlerPrinter : public IoTraceEventHandler {
public:
    IoTraceEventHandlerPrinter(const std::string &tracePath,
                               proto::OutputFormat format)
            : IoTraceEventHandler(tracePath)
            , m_table()
            , m_format(format)
            , m_jsonOptions()
            , m_jsonTrace() {
        m_jsonOptions.always_print_primitive_fields = true;
    }

    virtual ~IoTraceEventHandlerPrinter() = default;

    void handleIO(proto::trace::ParsedEvent &io) override {
        switch (m_format) {
        case proto::OutputFormat::CSV: {
            m_table[0].clear();
            m_table[0] << io;
            std::cout << m_table << std::endl;
        } break;

        case proto::OutputFormat::JSON: {
            m_jsonTrace.clear();
            google::protobuf::util::MessageToJsonString(io, &m_jsonTrace,
                                                        m_jsonOptions);
            std::cout << m_jsonTrace << std::endl;
        } break;

        default: { throw Exception("Invalid output format"); } break;
        }
    }

    void processEvents() override {
        if (m_format == proto::OutputFormat::CSV) {
            // Form CSV header and print it
            proto::trace::ParsedEvent event;
            table::setHeader(m_table[0], &event);
            std::cout << m_table << std::endl;
        }

        TraceEventHandler<proto::trace::Event>::processEvents();
    }

private:
    table::Table m_table;
    proto::OutputFormat m_format;
    google::protobuf::util::JsonOptions m_jsonOptions;
    std::string m_jsonTrace;
};

void InterfaceTraceParsingImpl::ParseTrace(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::ParseTraceRequest *request,
        ::octf::proto::Void *response,
        ::google::protobuf::Closure *done) {
    (void) (response);

    try {
        if (request->raw()) {
            if (request->format() == proto::OutputFormat::JSON) {
                IoTraceEventHandlerJsonPrinter parser(request->tracepath());
                parser.processEvents();
            } else if (request->format() == proto::OutputFormat::CSV) {
                IoTraceEventHandlerCsvPrinter parser(request->tracepath());
                parser.processEvents();
            } else {
                throw Exception("Invalid output format");
            }
        } else {
            IoTraceEventHandlerPrinter parser(request->tracepath(),
                                              request->format());
            parser.processEvents();
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
