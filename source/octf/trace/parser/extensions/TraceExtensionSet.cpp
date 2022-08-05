/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/extensions/TraceExtensionSet.h>

#include <octf/utils/Exception.h>

namespace octf {

TraceExtensionSet::TraceExtensionEntry::TraceExtensionEntry(
        TraceExtensionShRef extention)
        : ext(extention)
        , sid(ext->getReader().getNextSid()) {}

bool TraceExtensionSet::TraceExtensionEntry::operator<(
        const TraceExtensionEntry &other) const {
    if (sid != other.sid) {
        return sid < other.sid;
    }

    return ext->getName() < other.ext->getName();
}

void TraceExtensionSet::addTraceExtension(TraceExtensionShRef ext) {
    if (!ext->getReader().hasNext()) {
        // No trace extension to read, just don't insert it
        return;
    }

    TraceExtensionEntry entry(ext);
    m_set.insert(entry);
}

const std::string &TraceExtensionSet::getName() const {
    if (m_set.empty()) {
        throw Exception("ERROR, No more trace extension");
    }

    return m_set.begin()->ext->getName();
};

bool TraceExtensionSet::isWritable() const {
    return false;
};

bool TraceExtensionSet::isReady() const {
    for (const auto &entry : m_set) {
        if (!entry.ext->isReady()) {
            return false;
        }
    }

    return true;
}

void TraceExtensionSet::remove() {
    throw Exception("ERROR, Extension trace doesn't support removing");
}

ITraceExtension::ITraceExtensionWriter &TraceExtensionSet::getWriter() {
    throw Exception("ERROR, Extension set is not writable");
}

ITraceExtension::ITraceExtensionReader &TraceExtensionSet::getReader() {
    return *this;
}

void TraceExtensionSet::read(uint64_t &sid, google::protobuf::Message &ext) {
    TraceExtensionEntry next = *m_set.begin();
    m_set.erase(m_set.begin());

    next.ext->getReader().read(sid, ext);

    if (next.ext->getReader().hasNext()) {
        next.sid = next.ext->getReader().getNextSid();
        m_set.insert(next);
    }
};

bool TraceExtensionSet::hasNext() {
    return !m_set.empty();
};

uint64_t TraceExtensionSet::getNextSid() {
    if (!hasNext()) {
        throw Exception("ERROR, No more trace extension");
    }

    return m_set.begin()->ext->getReader().getNextSid();
};

}  // namespace octf
