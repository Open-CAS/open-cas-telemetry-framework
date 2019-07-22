/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <sstream>
#include <vector>
#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/CLIList.h>
#include <octf/cli/internal/cmd/ICommand.h>
#include <octf/cli/internal/param/ParamHelp.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {
namespace cli {

CLIList::CLIList()
        : m_list()
        , m_iter(m_list.begin()) {}

void CLIList::addElement(CLIElement element) {
    m_list.push_back(element);
}

CLIList &CLIList::operator+=(CLIElement element) {
    addElement(element);
    return *this;
}

void CLIList::create(const vector<string> &arguments) {
    m_list.clear();

    // Ommit program name, and add all CLI input
    for (unsigned int i = 1; i < arguments.size(); i++) {
        addElement(CLIElement(arguments[i]));
    }
    m_iter = m_list.begin();
}

void CLIList::create(string lines) {
    m_list.clear();

    stringstream ss(lines);
    string param;

    while (::std::getline(ss, param, ' ')) {
        if (param == "") {
            continue;
        }

        addElement(CLIElement(param));
    }

    m_iter = m_list.begin();
}

bool CLIList::hasNext() {
    if (m_iter != m_list.end()) {
        return true;
    } else {
        return false;
    }
}

const string &CLIList::next() {
    if (hasNext()) {
        CLIElement &e = *m_iter;
        m_iter++;
        return e.getValue();
    } else {
        static const string empty;
        return empty;
    }
}

CLIElement CLIList::nextElement() {
    if (hasNext()) {
        CLIElement e = *m_iter;
        m_iter++;
        return e;
    } else {
        static const string empty;
        return CLIElement(empty);
    }
}

bool CLIList::hasHelp() {
    if (hasNext()) {
        ParamHelp ph;

        const auto key = m_iter->getValidKeyName();

        if (key == ph.getLongKey() || key == ph.getShortKey()) {
            return true;
        }
    }

    return false;
}

}  // namespace cli
}  // namespace octf
