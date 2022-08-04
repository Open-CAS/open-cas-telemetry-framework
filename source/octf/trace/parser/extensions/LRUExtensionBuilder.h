/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDER_H
#define SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDER_H

#include <list>
#include <map>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/extensions.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>
#include <octf/trace/parser/extensions/ParsedIoExtensionBuilder.h>

namespace octf {

/**
 * @brief Builds simulated LRU cache hits
 * LRU cache algorithm is composed from two data structures
 * raw pointer doubly-linked list for storing cache
 * hashmap which allows O(1) random access to doubly-linked list
 */
class LRUExtensionBuilder : public ParsedIoExtensionBuilder {
public:
    /**
     * @param worksetSize - Size of working set ot the IO trace
     * @param cachePercentage - Sets cache size to the given percentage of
     * working set size
     */
    LRUExtensionBuilder(uint64_t worksetSize, uint64_t cachePercentage);
    virtual ~LRUExtensionBuilder();

    const std::string &getName() const override;

    MessageShRef getExtensionMessagePrototype() override;

    bool isTraceExtensionReady() override;

    const google::protobuf::Message &getTraceExtension() override;

private:
    bool isEventValid(const proto::trace::ParsedEvent &event);

    class LRUList {
    public:
        class Node {
        public:
            uint64_t lba = 0;
            Node *prev = nullptr;
            Node *next = nullptr;
        };

        Node *head = nullptr;
        Node *tail = nullptr;

        // Removes node from the list
        void pop(Node *node);
        // Creates new node on the tail
        Node *push(uint64_t lba);
        void push(Node *nnode);

        ~LRUList();
    };

    // LRU functions
    bool get(uint64_t lba);
    void push(uint64_t lba);
    // Removes least used element from the cache and lookup table
    void evict();

private:
    std::string m_name;
    uint64_t m_cacheLines;
    std::unordered_map<uint64_t, LRUList::Node> m_lookup;
    LRUList m_lru;
    proto::trace::TraceExtensionResult m_result;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_EXTENSIONS_LRUEXTENSIONBUILDER_H
