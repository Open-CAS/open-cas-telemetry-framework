/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/param/Parameter.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <octf/cli/internal/CLIUtils.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {
namespace cli {

Parameter::Parameter()
        : IParameter()
        , NonCopyable()
        , m_shortKey("")
        , m_longKey("")
        , m_desc("")
        , m_what("<VALUE>")
        , m_index()
        , m_set(false)
        , m_required(false)
        , m_hidden(false) {}

void Parameter::getHelp(stringstream &ss) const {
    if (isHidden()) {
        return;
    }

    string shortKey = getShortKey();
    string longKey = getLongKey();
    string desc = getDesc();
    string what = getWhat();

    if ("" != what) {
        longKey += (" " + what);
    }

    utils::printKeys(ss, shortKey, longKey, desc, false);

    return;
}

bool Parameter::isRequired() const {
    return m_required;
}

void Parameter::setRequired(bool required) {
    m_required = required;
}

bool Parameter::isValueSet() const {
    return m_set;
}

const string &Parameter::getShortKey() const {
    return m_shortKey;
}

void Parameter::setShortKey(const string &key) {
    m_shortKey = key;
}

const string &Parameter::getLongKey() const {
    return m_longKey;
}

void Parameter::setLongKey(const string &key) {
    m_longKey = key;
}

const string &Parameter::getDesc() const {
    return m_desc;
}

void Parameter::setDesc(const string &desc) {
    m_desc = desc;
}

const int32_t &Parameter::getIndex() const {
    return m_index;
}

void Parameter::setIndex(const int32_t index) {
    m_index = index;
}

const string &Parameter::getWhat() const {
    return m_what;
}

bool Parameter::hasValue() const {
    return true;
}

void Parameter::setWhat(const string &what) {
    m_what = what;
}

bool Parameter::isHidden() const {
    return m_hidden;
}

void Parameter::setHidden(bool hidden) {
    m_hidden = hidden;
}

bool Parameter::hasDefaultValue() const {
    return false;
}

void Parameter::setValueSet() {
    m_set = true;
}

void Parameter::setOptions(const google::protobuf::FieldDescriptor *fieldDesc) {
    const google::protobuf::FieldOptions &fieldOptions = fieldDesc->options();
    if (fieldOptions.HasExtension(proto::opts_param)) {
        setHidden(false);
        setRequired(
                fieldOptions.GetExtension(proto::opts_param).cli_required());
        setDesc(fieldOptions.GetExtension(proto::opts_param).cli_desc());

        // TODO(trybicki): Check if long key is valid
        setLongKey(fieldOptions.GetExtension(proto::opts_param).cli_long_key());
        // TODO(trybicki): Check if short key is valid
        setShortKey(
                fieldOptions.GetExtension(proto::opts_param).cli_short_key());
    }
}

bool Parameter::isMultipleValue() const {
    return false;
}

void Parameter::setMultipleValue(bool __attribute__((__unused__))
                                 multipleValue) {}

}  // namespace cli
}  // namespace octf
