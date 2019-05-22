/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/CLIProperties.h>

using namespace std;

CLIProperties::CLIProperties()
        : m_description()
        , m_name()
        , m_version() {}

CLIProperties::~CLIProperties() {}

CLIProperties &CLIProperties::getCliProperties() {
    static CLIProperties props;
    return props;
}

const string &CLIProperties::getDescription() const {
    return m_description;
}

void CLIProperties::setDescription(const string &description) {
    m_description = description;
}

const string &CLIProperties::getName() const {
    return m_name;
}

void CLIProperties::setName(const string &title) {
    m_name = title;
}

const string &CLIProperties::getVersion() const {
    return m_version;
}

void CLIProperties::setVersion(const string &version) {
    m_version = version;
}
