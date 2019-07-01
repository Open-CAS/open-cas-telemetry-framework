/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/cli/internal/cmd/CmdVersion.h>

#include <octf/cli/CLIProperties.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {
namespace cli {

CmdVersion::CmdVersion(const CLIProperties &properties)
        : Command()
        , m_cliProperties(properties) {
    setShortKey("V");
    setLongKey("version");
    setDesc("Print version");
    setLocal(true);
}

void CmdVersion::execute() {
    log::cout << m_cliProperties.getVersion() << endl;

    std::string oldPrefix = log::cout.getPrefix();
    log::cout << log::prefix << "OCTF"
              << getFrameworkConfiguration().getVersion() << endl;
    log::cout << log::prefix << oldPrefix;
}

}  // namespace cli
}  // namespace octf
