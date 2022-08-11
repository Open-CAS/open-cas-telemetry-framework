/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/extensions/LRUExtensionBuilder.h>
#include <octf/utils/SizeConversion.h>

namespace octf {

LRUExtensionBuilder::LRUExtensionBuilder(uint64_t worksetSize,
                                         uint64_t cachePercentage)
        : ParsedIoExtensionBuilder()
        , m_name()
        , m_cacheLines()
        , m_lookup()
        , m_lru()
        , m_result()
        , m_noRq()
        , m_noHit() {
    m_name = "LRU " + std::to_string(cachePercentage) + "%";

    m_cacheLines = sectorToCacheLine(worksetSize) * cachePercentage / 100ULL;
    log::verbose << "LRU builder configuration, cache percentage "
                 << cachePercentage << ", cache lines " << m_cacheLines
                 << ", working set " << sectorToCacheLine(worksetSize)
                 << std::endl;

    BuildStepEventHandler hndlr = [this](const proto::trace::ParsedEvent &e) {
        if (!isEventValid(e)) {
            m_result.Clear();
            return true;
        }

        uint64_t len = e.io().len();
        uint64_t lba = e.io().lba();

        uint64_t firstCacheLine = sectorToCacheLine(lba);
        uint64_t lastCacheLine = sectorToCacheLine(lba + len - 1);

        // cache will be hit if all cache lines in request were hit
        bool hit = true;

        for (uint64_t i = firstCacheLine; i <= lastCacheLine; i++) {
            hit &= get(i);
            push(i);
        }

        m_noRq++;
        if (hit) {
            m_noHit++;
        }

        // Construct result message
        m_result.mutable_cache()->set_hit(hit);
        m_result.mutable_cache()->set_hitratio(m_noHit * 100ULL / m_noRq);
        return true;
    };
    addBuildStepEventHandler(hndlr);
}

LRUExtensionBuilder::~LRUExtensionBuilder() {}

bool LRUExtensionBuilder::isEventValid(const proto::trace::ParsedEvent &event) {
    if (!event.has_io()) {
        return false;
    }

    auto op = event.io().operation();
    if (proto::trace::Read != op && proto::trace::Write != op) {
        return false;
    }

    if (0 == event.io().len()) {
        return false;
    }

    return true;
}

const std::string &LRUExtensionBuilder::getName() const {
    return m_name;
}

MessageShRef LRUExtensionBuilder::getExtensionMessagePrototype() {
    auto prototype = std::make_shared<proto::trace::TraceExtensionResult>();
    prototype->mutable_cache()->set_hit(false);
    return prototype;
}

bool LRUExtensionBuilder::isTraceExtensionReady() {
    return true;
}

const google::protobuf::Message &LRUExtensionBuilder::getTraceExtension() {
    return m_result;
}

// Use std::move to move node ownership from free-list to lookup
void LRUExtensionBuilder::push(uint64_t lba) {
    auto iter = m_lookup.find(lba);
    if (iter != m_lookup.end()) {
        // If LBA was found in cache, move it to the most recently used position
        LRUList::Node *n = &iter->second;
        m_lru.pop(n);
        m_lru.push(n);
    } else {
        // If LBA was not found in teh cache, create new entry and evict if
        // capacity was exceeded
        if (m_lookup.size() >= m_cacheLines)
            evict();
        // Create node in lookup map
        auto result = m_lookup.emplace(std::make_pair(lba, LRUList::Node(lba)));
        m_lru.push(&(*result.first).second);
    }
}

bool LRUExtensionBuilder::get(uint64_t lba) {
    return m_lookup.count(lba) == 1;
}

void LRUExtensionBuilder::evict() {
    LRUList::Node *lru_node = m_lru.head;
    m_lookup.erase(lru_node->lba);
    m_lru.pop(lru_node);
}

void LRUExtensionBuilder::LRUList::pop(Node *node) {
    if (head == nullptr)
        return;  // list is empty, do nothing.

    // Update list main pointers if necessary
    if (tail == node)
        tail = tail->prev;

    if (head == node)
        head = head->next;

    // P<->C<->N ----> P<->N
    if (node->prev != nullptr)
        node->prev->next = node->next;

    if (node->next != nullptr)
        node->next->prev = node->prev;

    // detach
    node->prev = nullptr;
    node->next = nullptr;
}

void LRUExtensionBuilder::LRUList::push(Node *nnode) {
    if (head == nullptr) {
        // list is currently empty: fill initial values
        head = nnode;
        tail = nnode;
        return;
    }

    tail->next = nnode;
    nnode->prev = tail;
    tail = tail->next;
}

LRUExtensionBuilder::LRUList::~LRUList() {
    while (head != nullptr) {
        this->pop(head);
    }
}

}  // namespace octf
