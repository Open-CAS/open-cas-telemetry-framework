/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/CLIProperties.h>
#include <octf/cli/cmd/CmdVersion.h>
#include <octf/utils/FrameworkConfiguration.h>
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
    log::cout << "Version: " << CLIProperties::getCliProperties().getVersion()
              << endl;

    std::string oldPrefix = log::cout.getPrefix();

    log::cout << log::prefix << "OCTF"
              << getFrameworkConfiguration().getVersion() << endl;

    log::cout << log::prefix << oldPrefix;
}

}  // namespace octf
