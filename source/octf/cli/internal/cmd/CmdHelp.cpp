/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/cmd/CmdHelp.h>
#include <octf/utils/Log.h>

using namespace std;

namespace octf {
namespace cli {

CmdHelp::CmdHelp()
        : Command() {
    setShortKey("H");
    setLongKey("help");
    setDesc("Prints help");
    setLocal(true);
    setHelp("Help not available");
}

void CmdHelp::execute() {
    log::cout << m_help;
}

void CmdHelp::setHelp(const std::string &help) {
    m_help = help;
}

}  // namespace cli
}  // namespace octf
