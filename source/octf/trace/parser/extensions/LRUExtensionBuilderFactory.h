/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDERFACTORY_H
#define SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDERFACTORY_H

#include <list>
#include <octf/trace/parser/extensions/ParsedIoExtensionBuilderFactory.h>

namespace octf {

class LRUExtensionBuilderFactory : public IParsedIoExtensionBuilderFactory {
public:
    LRUExtensionBuilderFactory() = default;
    ~LRUExtensionBuilderFactory() = default;
    std::list<std::shared_ptr<ParsedIoExtensionBuilder>> createBuilders(
            const std::string &tracepath) override;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDERFACTORY_H