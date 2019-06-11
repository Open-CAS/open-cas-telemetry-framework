/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/param/ParamHelp.h>
#include <octf/cli/internal/param/Parameter.h>

namespace octf {

ParamHelp::ParamHelp()
        : Parameter() {
    setShortKey("H");
    setLongKey("help");
}

void ParamHelp::setValue(CLIElement value) {
    // Dummy instruction to avoid compilation warning
    // value not relevant here
    (void) value;

    setValueSet();
}

bool ParamHelp::hasValue() const {
    return false;
}

}  // namespace octf
