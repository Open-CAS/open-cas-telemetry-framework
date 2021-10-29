/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sstream>
#include <string>
#include <octf/cli/internal/Module.h>

using namespace std;

namespace octf {
namespace cli {

Module::Module()
        : m_shortKey("")
        , m_longKey("")
        , m_desc("")
        , m_isLocal(false) {}

Module::Module(const Module &other)
        : m_shortKey(other.m_shortKey)
        , m_longKey(other.m_longKey)
        , m_desc(other.m_desc)
        , m_isLocal(other.m_isLocal) {}

Module &Module::operator=(const Module &other) {
    if (&other != this) {
        m_desc = other.m_desc;
        m_shortKey = other.m_shortKey;
        m_longKey = other.m_longKey;
        m_isLocal = other.m_isLocal;
    }

    return *this;
}

bool Module::operator<(const Module &other) const {
    if (m_shortKey != other.m_shortKey) {
        return m_shortKey < other.m_shortKey;
    } else {
        return m_longKey < other.m_longKey;
    }
}

bool Module::operator==(const Module &other) const {
    return (m_shortKey == other.m_shortKey) && (m_longKey == other.m_longKey);
}

const string &Module::getLongKey() const {
    return m_longKey;
}

void Module::setLongKey(const string &longKey) {
    m_longKey = longKey;
}

const std::string &Module::getShortKey() const {
    return m_shortKey;
}

void Module::setShortKey(const string &shortKey) {
    m_shortKey = shortKey;
}

const string &Module::getDesc() const {
    return m_desc;
}

void Module::setDesc(const std::string &desc) {
    m_desc = desc;
}

bool Module::isLocal() const {
    return m_isLocal;
}

void Module::setLocal(bool isLocal) {
    this->m_isLocal = isLocal;
}

}  // namespace cli
}  // namespace octf
