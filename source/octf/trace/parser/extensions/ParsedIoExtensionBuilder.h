/*
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDER_H
#define SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDER_H

#include <octf/interface/ITraceExtensionBuilder.h>

namespace octf {

class ParsedIoExtensionBuilder
        : public ITraceExtensionBuilder<proto::trace::ParsedEvent> {
public:
    ParsedIoExtensionBuilder() = default;
    virtual ~ParsedIoExtensionBuilder() = default;

    const std::list<BuildStepEventHandler> &getBuildProcess() override {
        return m_stepEventHandlerList;
    }

protected:
    void addBuildStepEventHandler(BuildStepEventHandler handler) {
        m_stepEventHandlerList.push_back(handler);
    }

private:
    std::list<BuildStepEventHandler> m_stepEventHandlerList;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDER_H
