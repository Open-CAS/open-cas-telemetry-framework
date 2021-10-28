/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sstream>
#include <octf/utils/CsvParser.h>
#include <octf/utils/Exception.h>

namespace octf {
namespace csvparser {

using namespace std;

void parseCsv(const std::string &content,
              const std::list<std::string> &requiredHeaderElements,
              table::Table &parsed) {
    stringstream ss;
    ss << content;

    parseCsv(ss, requiredHeaderElements, parsed);
}

void parseCsv(std::istream &content,
              const std::list<std::string> &requiredHeaderElements,
              table::Table &parsed) {
    using namespace ::octf::table;

    parsed.clear();
    Row &header = parsed[0];

    // Check stream size
    int originalstreamPos = content.tellg();
    content.seekg(0, ios::end);
    int streamSize = content.tellg();

    // Get last real (non-zero) char. Do this because CSV content can come
    // from a 0 terminated string.
    content.seekg(-1, ios::end);
    char last = 0;
    int pos = content.tellg();
    while (last == 0 && pos > 0) {
        content.get(last);
        content.seekg(-1, ios::cur);
        pos = content.tellg();
    }

    // Get number of CSV lines by counting \n chars. However content may have
    // a last line without a \n, so we need to check the last char to verify.
    content.seekg(originalstreamPos, ios::beg);
    unsigned int lineCount = std::count(istreambuf_iterator<char>(content),
                                        istreambuf_iterator<char>(), '\n');
    content.seekg(originalstreamPos, ios::beg);

    // Last line does not have a newline at the end
    if (last != '\n') {
        lineCount++;
    }

    if (streamSize > MAX_CSV_CONTENT_SIZE) {
        throw Exception("Input content for CSV parser is too large.");
    } else if (streamSize == 0 || lineCount <= 1) {
        throw Exception("CSV content empty or no records found");
    }

    // Parse header first
    {
        list<string> headerList;
        string token = "";
        string line;
        std::getline(content, line);
        stringstream lineSS(line);

        // Parse the header, delimiting tokens by comma
        while (std::getline(lineSS, token, ',')) {
            unsigned int hdrCount = header.size();
            Cell &headerElement = header[token];

            if (header.getAddress(token).getColumnIndex() < hdrCount) {
                throw Exception("Duplicate header elements found in CSV: " +
                                token);
            } else {
                headerElement = token;
            }
        }

        if (!lineSS && token.empty()) {
            // Trailing comma with no data after it - empty hdr element
            throw Exception("Empty header element found during CSV parsing.");
        }

        // Empty header
        if (header.empty()) {
            throw Exception(
                    "Invalid CSV format found during parsing: "
                    "no headers found.");
        }
    }

    // Verify header contains required elements:
    for (const auto &required : requiredHeaderElements) {
        bool matched = false;
        for (const auto &hdrElem : header) {
            if (hdrElem == required) {
                matched = true;
                break;
            }
        }

        if (matched == false) {
            throw Exception("All required header elements were not found.");
        }
    }

    // Parse records line by line
    string line;
    unsigned int lineIndex = 1;
    unsigned int tokenIndex = 0;

    while (std::getline(content, line)) {
        if (lineIndex >= lineCount) {
            throw Exception("Unexpected error occurred - line count mismatch");
        }

        stringstream lineSS(line);
        string token = "";

        // Parse line by token delimited by comma
        while (std::getline(lineSS, token, ',')) {
            if (tokenIndex >= header.size()) {
                throw Exception(
                        "Records without header found during CSV parsing.");
            }

            parsed[lineIndex][tokenIndex] = token;
            tokenIndex++;
        }

        if (!lineSS && token.empty()) {
            // Trailing comma with no data after it.
            parsed[lineIndex][header.size() - 1] = "";
        }

        lineIndex++;
    }
}

}  // namespace csvparser
}  // namespace octf
