/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <vector>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/TraceEventHandlerWorkset.h>
#include <octf/trace/parser/extensions/LRUExtensionBuilder.h>
#include <octf/trace/parser/extensions/LRUExtensionBuilderFactory.h>

namespace octf {

std::list<std::shared_ptr<ParsedIoExtensionBuilder>>
LRUExtensionBuilderFactory::createBuilders(const std::string &tracepath) {
    // RpcOutputStream verbose(log::Severity::Verbose, controller);
    auto trace = TraceLibrary::get().getTrace(tracepath);
    auto &cache = trace->getCache();
    uint64_t workset = 0;

    if (!cache.read("BuildExtensionsWorkset", workset)) {
        // TODO: add logging
        /* No cached result, perform required processing */
        CasTraceEventHandlerWorkset handler(tracepath);
        handler.processEvents();
        workset = handler.getWorkset();
        cache.write("BuildExtensionsWorkset", workset);
    }

    std::list<std::shared_ptr<ParsedIoExtensionBuilder>> result;
    const std::vector<uint64_t> percentages{5, 10, 15, 20, 50, 100};
    for (auto prcnt : percentages) {
        auto builder = std::make_shared<LRUExtensionBuilder>(workset, prcnt);
        result.push_back(builder);
    }

    return result;
}

}  // namespace octf
