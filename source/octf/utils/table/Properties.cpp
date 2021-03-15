/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Properties.h>

namespace octf {
namespace table {

Properties::Builtins builtin;

Properties::Properties()
        : format(TableFormat::csv)
        , border(BorderStyle::empty)
        , header(false) {}

Properties::~Properties() {}

Properties::Properties(const Properties &other)
        : format(other.format)
        , border(other.border)
        , header(other.header) {}

Properties &Properties::operator=(const Properties &other) {
    if (&other == this) {
        return *this;
    }

    format = other.format;
    border = other.border;
    header = other.header;

    return *this;
}

const Properties &octf::table::Properties::Builtins::csv() {
    struct Style : Properties {
        Style() {
            format = TableFormat::csv;
            border = BorderStyle::empty;
            header = true;
        }
        virtual ~Style() = default;
    } const static style;

    return style;
};

const Properties &octf::table::Properties::Builtins::empt() {
    struct Style : Properties {
        Style() {
            format = TableFormat::text;
            border = BorderStyle::empty;
            header = false;
        }
        virtual ~Style() = default;
    } const static style;

    return style;
}

const Properties &octf::table::Properties::Builtins::solid() {
    struct Style : Properties {
        Style() {
            format = TableFormat::text;
            border = BorderStyle::solid;
            header = false;
        }
        virtual ~Style() = default;
    } const static style;

    return style;
}

}  // namespace table
}  // namespace octf
