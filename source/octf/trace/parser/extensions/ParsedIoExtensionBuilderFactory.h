/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDERFACTORY_H
#define SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDERFACTORY_H

#include <list>
#include <memory>
#include <octf/trace/parser/extensions/ParsedIoExtensionBuilder.h>

namespace octf {

class IParsedIoExtensionBuilderFactory {
public:
    virtual std::list<std::shared_ptr<ParsedIoExtensionBuilder>> createBuilders(
            const std::string &tracepath) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_PARSEDIOEXTENSIONBUILDERFACTORY_H