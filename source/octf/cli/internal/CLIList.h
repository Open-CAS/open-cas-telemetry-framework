/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_CLILIST_H
#define SOURCE_OCTF_CLI_INTERNAL_CLILIST_H

#include <list>
#include <vector>
#include <octf/cli/internal/CLIElement.h>
#include <octf/cli/internal/CommandSet.h>

namespace octf {
namespace cli {

/**
 * @brief List of CLIElements, excluding application name
 */
class CLIList {
public:
    /**
     * @brief Default constructor
     */
    CLIList();

    virtual ~CLIList() = default;

    /**
     * @brief Creates CLIList
     * @param argv Vector with arguments
     */
    void create(const std::vector<std::string> &argv);

    /**
     * @brief Creates CLIList
     * @param lines String containing space separated CLI elements
     */
    void create(std::string lines);

    /**
     * @brief Adds CLIElement to CLIList
     * @param element CLIElement to be added
     * @return CLIList
     */
    CLIList &operator+=(CLIElement element);

    /**
     * @brief Adds CLI element to CLI list
     * @param element CLIElement to be added
     */
    void addElement(CLIElement element);

    /**
     * @brief Checks if there are still CLILists elements to iterate
     * @retval true There are not iterated elements
     * @retval false All CLIList elements have been iterated
     */
    bool hasNext();

    /**
     * @brief Gets next CLIList element
     * @return CLI element as string
     */
    const std::string &next();

    CLIElement nextElement();

private:
    std::list<CLIElement> m_list;
    std::list<CLIElement>::iterator m_iter;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_CLILIST_H
