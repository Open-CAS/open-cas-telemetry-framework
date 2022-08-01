#include <octf/trace/parser/extensions/LRUExtensionBuilder.h>
#include <octf/utils/SizeConversion.h>

namespace octf {

LRUExtensionBuilder::LRUExtensionBuilder(uint64_t workset_size,
                                         uint64_t cache_percentage)
        : result_table()
        , result_message()
        , lookup()
        , cache() {
    this->workset_size = workset_size;
    this->cache_size = workset_size * (float) (cache_percentage / 100.0);
}

LRUExtensionBuilder::~LRUExtensionBuilder() {}

const google::protobuf::Message &LRUExtensionBuilder::GetMessage() {
    return result_message;
}

octf::table::Table LRUExtensionBuilder::buildExtension() {
    return this->result_table;
}

const google::protobuf::Message &LRUExtensionBuilder::handleIO(
        const proto::trace::ParsedEvent &io) {
    uint64_t len = io.io().len();
    uint64_t lba = io.io().lba();

    uint64_t firstCacheLine = sectorToCacheLine(lba);
    uint64_t lastCacheLine = sectorToCacheLine(lba + len - 1);
    bool hit = true;  // cache will be hit if all cachelines in request were hit
    for (uint64_t i = firstCacheLine; i <= lastCacheLine; i++) {
        hit &= get(i);
        push(i);
    }

    // Construct result message
    result_message.set_hit(hit);
    return result_message;
}

// Use std::move to move node ownership from free-list to lookup
void LRUExtensionBuilder::push(uint64_t lba) {
    auto iter = lookup.find(lba);
    if (iter != lookup.end()) {
        // If LBA was found in cache, move it to the most recently used position
        LRUList::Node *n = &iter->second;
        cache.pop(n);
        cache.push(n);
    } else {
        // If LBA wasnt found in cache, create new entry and evict if
        // capacity was exceeded
        if (lookup.size() >= cache_size)
            evict();
        // Create node in lookup map
        auto result = lookup.emplace(std::make_pair(lba, LRUList::Node()));
        cache.push(&(*result.first).second);
    }
}

bool LRUExtensionBuilder::get(uint64_t lba) {
    return lookup.count(lba) == 1;
}

void LRUExtensionBuilder::evict() {
    LRUList::Node *lru_node = cache.head;
    lookup.erase(lru_node->lba);
    cache.pop(lru_node);
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
