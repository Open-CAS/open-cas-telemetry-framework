/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/param/ParamYesNo.h>

namespace octf {

ParamYesNo::ParamYesNo()
        : ParamEnum() {
    addEnumVal(No, "no", "");
    addEnumVal(Yes, "yes", "");
}

}  // namespace octf
