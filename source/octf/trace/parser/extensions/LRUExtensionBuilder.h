/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_LRUBUILDER_H
#define SOURCE_OCTF_TRACE_PARSER_LRUBUILDER_H

#include <list>
#include <map>
#include <octf/interface/ITraceExtensionBuilder.h>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/trace.pb.h>
#include <octf/trace/parser/ParsedIoTraceEventHandler.h>

namespace octf {

/**
 * @brief Builds simulated LRU cache hits
 * LRU cache algorithm is composed from two data structures
 * raw pointer doubly-linked list for storing cache
 * hashmap which allows O(1) random access to doubly-linked list
 */
class LRUExtensionBuilder : public ITraceExtensionBuilder {
public:
    /**
     * @see TraceEventHandler
     */
    LRUExtensionBuilder(uint64_t workset_size);

    virtual ~LRUExtensionBuilder();

    virtual void buildExtension(const proto::trace::ParsedEvent &io) override;
    virtual void serializeExtension(
            const proto::trace::ParsedEvent &io) override;

private:
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

    uint64_t sector_size = 4096;
    uint64_t workset_size = 0;
    uint64_t cache_size = 0;
    std::unordered_map<uint64_t, LRUList::Node *> lookup;
    LRUList cache;

    // LRU functions
    void get(uint64_t lba);
    void push(uint64_t lba);
    // Removes least used element from the cache and lookup table
    void evict();
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_LRUBUILDER_H
