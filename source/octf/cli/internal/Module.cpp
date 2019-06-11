/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <sstream>
#include <string>
#include <octf/cli/internal/Module.h>

using namespace std;

namespace octf {

Module::Module()
        : m_shortKey("")
        , m_longKey("")
        , m_desc("")
        , m_isLocal(false) {}

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

Module &Module::operator=(const Module &other) {
    m_desc = other.getDesc();
    m_shortKey = other.getShortKey();
    m_longKey = other.getLongKey();
    m_isLocal = other.isLocal();

    return *this;
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

}  // namespace octf
