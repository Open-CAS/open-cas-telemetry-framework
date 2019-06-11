/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/cmd/CmdVersion.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {

CmdVersion::CmdVersion(const CLIProperties &properties)
        : Command()
        , m_cliProperties(properties) {
    setShortKey("V");
    setLongKey("version");
    setDesc("Print version");
    setLocal(true);
}

void CmdVersion::execute() {
    log::cout << "Name: " << m_cliProperties.getName() << endl;
    log::cout << "Version: " << m_cliProperties.getVersion() << endl;
}

}  // namespace octf
