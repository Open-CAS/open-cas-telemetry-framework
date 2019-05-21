/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/CLIProperties.h>
#include <octf/cli/cmd/CmdVersion.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {

CmdVersion::CmdVersion()
        : Command() {
    setShortKey("V");
    setLongKey("version");
    setDesc("Print version");
    setLocal(true);
}

void CmdVersion::execute() {
    log::cout << "Name: " << CLIProperties::getCliProperties().getName()
              << endl;
    log::cout << "Version: " << CLIProperties::getCliProperties().getVersion()
              << endl;
}

}  // namespace octf
