/*
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/trace/parser/ParsedIoTraceEventHandlerExtensionBuilder.h>

namespace octf {

ParsedIoTraceEventHandlerExtensionBuilder::
        ParsedIoTraceEventHandlerExtensionBuilder(
                const std::string &tracePath,
                std::shared_ptr<ParsedIoExtensionBuilder> builder)
        : ParsedIoTraceEventHandler(tracePath)
        , m_builder(builder)
        , m_handler()
        , m_trace(TraceLibrary::get().getTrace(tracePath))
        , m_traceExt() {}

void ParsedIoTraceEventHandlerExtensionBuilder::handleIO(
        const proto::trace::ParsedEvent &io) {
    bool result = m_handler(io);

    if (m_builder->isTraceExtensionReady()) {
        auto &writer = m_traceExt->getWriter();
        writer.write(io.header().sid(), m_builder->getTraceExtension());
    }

    if (!result) {
        // Handler requested to stop processing
        cancel();
    }
}

void ParsedIoTraceEventHandlerExtensionBuilder::processEvents() {
    // Initialize trace extension
    m_traceExt = m_trace->getExtension(m_builder->getName());
    if (m_traceExt->isWritable()) {
        log::verbose << "Building extension for: " << m_builder->getName()
                     << std::endl;

        const auto &steps = m_builder->getBuildProcess();
        if (steps.empty()) {
            throw Exception("Builder does not define building steps");
        }

        for (const auto &step : steps) {
            m_handler = step;

            // Reinitialize parser
            deinitParser();
            initParser();

            // Start parsing
            ParsedIoTraceEventHandler::processEvents();
        }

        // Processing finished, commit trace extension
        m_traceExt->getWriter().commit();
    } else {
        log::verbose << "Read extension from the cache: "
                     << m_builder->getName() << std::endl;
    }
}

}  // namespace octf
