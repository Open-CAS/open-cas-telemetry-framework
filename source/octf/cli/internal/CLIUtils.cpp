/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

using namespace std;

#include <octf/cli/internal/CLIUtils.h>

#include <google/protobuf/util/json_util.h>
#include <exception>
#include <iomanip>
#include <sstream>
#include <octf/cli/CLIProperties.h>
#include <octf/cli/internal/Module.h>
#include <octf/cli/internal/cmd/ICommand.h>
#include <octf/utils/Exception.h>
#include <octf/utils/OptionsValidation.h>

using namespace std;

namespace octf {

namespace cliUtils {

string bytes2string(uint64_t bytes, uint64_t base) {
    stringstream ss;
    double number = bytes;
    double factor = KiB;

    if (!base) {
        while (number / factor > 1024) {
            factor *= KiB;
        }
    } else {
        factor = base;
    }

    ss << fixed << setprecision(2) << number / factor << " ";

    if (!base) {
        ss << to_string(factor);
    }

    return ss.str();
}

void printProgressBar(double prog, ostream &out) {
    char end;
    int width = 70;
    string line = "";

    out << '\r';

    line += "[";
    int pos = width * prog;
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            line += "=";
        } else {
            line += " ";
        }
    }

    line += "] ";

    if (1.0 == prog) {
        end = '\n';
    } else {
        end = '\0';
    }

    out << line << int(prog * 100.0) << " %" << end;
    out.flush();
}

string printProgressBar(double prog) {
    int width = 20;
    string line = "";

    line += "[";
    int pos = width * prog;
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            line += "=";
        } else {
            line += " ";
        }
    }

    line += "] ";

    stringstream out;
    out << line << int(prog * 100.0) << " %";

    return out.str();
}

string double2string(double f, uint32_t precision) {
    stringstream ss;

    ss << fixed << setprecision(precision) << f;

    return ss.str();
}

void printKeys(stringstream &ss,
               string shortKey,
               string longKey,
               string desc,
               bool isList) {
    string leftBracket = "{";
    string rightBracket = "}";
    string delimiter = " | ";
    string ending = " ";
    int shortAlignment = 0;
    int longAlignment = 0;
    int emptyShortKeyAlignment = 0;

    if (isList) {
        leftBracket = "";
        rightBracket = "";
        delimiter = "    ";
        ending = "\n";
        shortAlignment = 5;
        longAlignment = 35;
        emptyShortKeyAlignment = shortAlignment + 8;
    }

    ss << std::setw(shortAlignment) << std::right;

    if (!shortKey.empty()) {
        ss << leftBracket << SHORT_KEY_PREFIX << shortKey << delimiter;
    } else {
        // Don't write right bracket if the left one was skipped
        rightBracket = "";
        // Compensate for empty short key
        ss << std::setw(emptyShortKeyAlignment) << std::right;
    }

    ss << LONG_KEY_PREFIX;
    ss << std::setw(longAlignment) << std::left;

    ss << longKey << rightBracket << " " << desc << ending;
}

void printOutputMessage(MessageShRef message) {
    google::protobuf::util::Status status;
    google::protobuf::util::JsonPrintOptions opts;
    string strOutput;

    // enable new lines in output for each new fields
    opts.add_whitespace = true;

    // Default printing configuration, when field has default value
    // (e.g. a number field set to 0), it's not printed, but we want to have
    // this field in output, enable this
    opts.always_print_primitive_fields = true;

    status = google::protobuf::util::MessageToJsonString(*message, &strOutput,
                                                         opts);
    if (!status.ok()) {
        throw ProtoBufferException(status.ToString());
    };

    // Because message output is formated in JSON we have to reset output
    // in order to prevent nested JSON.
    log::cout << log::reset << strOutput;
}

void printOutputString(const std::string &output) {
    log::cout << output << std::endl;
}

void printModuleHelp(stringstream &ss, Module *module, bool isList) {
    if (module) {
        cliUtils::printKeys(ss, module->getShortKey(), module->getLongKey(), "",
                            isList);
    }
}

void printUsage(stringstream &ss,
                Module *module,
                const CLIProperties &cliProperties,
                bool isList,
                bool hasPlugins) {
    ss << "Usage: " << cliProperties.getName() << " ";
    if (module) {
        // Specify keys supported by this module
        printModuleHelp(ss, module, isList);

    } else {
        if (hasPlugins) {
            ss << "[plugin] ";
        }
    }

    ss << "command [options...]" << std::endl;
}

void printCmdSetHelp(stringstream &ss, const CommandSet &cmdSet) {
    ss << std::endl << "Available commands: " << std::endl;
    cmdSet.getHelp(ss);
}

void printCmdHelp(stringstream &ss,
                  std::shared_ptr<ICommand> cmd,
                  const CLIProperties &cliProperties) {
    ss << "Usage: ";
    ss << cliProperties.getName() << " ";
    cmd->getCommandUsage(ss);
    ss << "\n\n" << cmd->getDesc() << std::endl;

    ss << endl;
    ss << "Options that are valid with ";
    cliUtils::printKeys(ss, cmd->getShortKey(), cmd->getLongKey(), "", false);
    ss << std::endl;
    cmd->getHelp(ss);
}

}  // namespace cliUtils
}  // namespace octf
