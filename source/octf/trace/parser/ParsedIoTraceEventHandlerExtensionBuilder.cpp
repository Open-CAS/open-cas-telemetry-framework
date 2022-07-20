#include <octf/trace/parser/ParsedIoTraceEventHandlerExtensionBuilder.h>

namespace octf {

ParsedIoTraceEventHandlerExtensionBuilder::
        ParsedIoTraceEventHandlerExtensionBuilder(
                const std::string &tracePath,
                ITraceExtensionBuilder *builder)
        : ParsedIoTraceEventHandler(tracePath) {
    this->builder = builder;
}

void ParsedIoTraceEventHandlerExtensionBuilder::handleIO(
        const proto::trace::ParsedEvent &io) {
    builder->buildExtension(io);
}

}  // namespace octf
