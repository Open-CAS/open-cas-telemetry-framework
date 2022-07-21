#include <octf/trace/parser/ParsedIoTraceEventHandlerExtensionBuilder.h>

namespace octf {

ParsedIoTraceEventHandlerExtensionBuilder::
        ParsedIoTraceEventHandlerExtensionBuilder(
                const std::string &tracePath,
                ITraceExtensionBuilder *builder,
                proto::OutputFormat format)
        : ParsedIoTraceEventHandler(tracePath)
        , m_trace(TraceLibrary::get().getTrace(tracePath))
        , m_table()
        , m_format(format)
        , m_jsonOptions()
        , m_jsonTrace() {
    this->builder = builder;
    m_jsonOptions.always_print_primitive_fields = true;
    m_jsonOptions.add_whitespace = false;
}

void ParsedIoTraceEventHandlerExtensionBuilder::handleIO(
        const proto::trace::ParsedEvent &io) {
    const google::protobuf::Message &message = builder->handleIO(io);

    switch (m_format) {
    case proto::OutputFormat::CSV: {
        m_table[0].clear();
        m_table[0] << message;
        std::cout << m_table << std::endl;
    } break;
    case proto::OutputFormat::JSON: {
        m_jsonTrace.clear();
        google::protobuf::util::MessageToJsonString(message, &m_jsonTrace,
                                                    m_jsonOptions);
        std::cout << m_jsonTrace << std::endl;
    } break;
    default: {
        throw Exception("Invalid output format");
    } break;
    }
}

void ParsedIoTraceEventHandlerExtensionBuilder::processEvents() {
    if (m_format == proto::OutputFormat::CSV) {
        const auto &message = builder->GetMessage();
        table::setHeader(m_table[0], &message);
        std::cout << m_table << std::endl;
    }

    ParsedIoTraceEventHandler::processEvents();
}
}  // namespace octf