#include <octf/trace/parser/extensions/LRUExtensionBuilder.h>

namespace octf {

LRUExtensionBuilder::LRUExtensionBuilder(uint64_t workset_size) {
    this->workset_size = workset_size;
    this->cache_size = workset_size*0.1;
    this->cache = LRUList();
    this->lookup = {};
}

LRUExtensionBuilder::~LRUExtensionBuilder() {

}

void LRUExtensionBuilder::buildExtension(const proto::trace::ParsedEvent &io) {
    //TODO: map adresses to 4KB blocks
    //TODO: go through adress blocks
    get(io.io().lba());
    push(io.io().lba());
}

void LRUExtensionBuilder::serializeExtension(
        const proto::trace::ParsedEvent &io) {}

void LRUExtensionBuilder::push(uint64_t lba) {
    auto iter = lookup.find(lba);
    if (iter != lookup.end()) {
        // If LBA was found in cache, move it to the most recently used position
        LRUList::Node *n = iter->second;
        cache.pop(n);
        cache.push(n);
    } else {
        // If LBA wasnt found in cache, create new entry and evict if
        // capacity was exceeded
        if (lookup.size() >= cache_size)
            evict();
        LRUList::Node *node = cache.push(lba);
        lookup.insert({lba, node});
    }
}

void LRUExtensionBuilder::get(uint64_t lba) {
    if(lookup.count(lba) == 1)
    {
        std::cout << "True" << std::endl;
    }
    else
    {
        std::cout << "False" << std::endl;
    }
}

void LRUExtensionBuilder::evict() {
    LRUList::Node *lru_node = cache.head;
    lookup.erase(lru_node->lba);
    cache.pop(lru_node);
    delete lru_node;
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

LRUExtensionBuilder::LRUList::Node *LRUExtensionBuilder::LRUList::push(
        uint64_t lba) {
    Node *nnode = new Node();
    nnode->lba = lba;
    push(nnode);
    return nnode;
}

LRUExtensionBuilder::LRUList::~LRUList() {
    while (head != nullptr) {
        this->pop(head);
    }
}

}  // namespace octf
