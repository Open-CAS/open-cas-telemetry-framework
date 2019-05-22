/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CLIPROPERTIES_H
#define SOURCE_OCTF_CLI_CLIPROPERTIES_H

#include <string>

/**
 * @brief Class managing properties of CLI
 */
class CLIProperties {
public:
    /**
     * @return static CLIProperties object
     */
    static CLIProperties &getCliProperties();

    /**
     * @return Description of CLI
     */
    const std::string &getDescription() const;

    /**
     * @param description Description of CLI
     */
    void setDescription(const std::string &description);

    /**
     * @return Title of CLI
     */
    const std::string &getName() const;

    /**
     * @param title Title of CLI
     */
    void setName(const std::string &title);

    /**
     * @return Version of CLI
     */
    const std::string &getVersion() const;

    /**
     * @param version Version of CLI
     */
    void setVersion(const std::string &version);

private:
    CLIProperties();
    virtual ~CLIProperties();
    CLIProperties &operator=(const CLIProperties &cliProperties);
    CLIProperties(const CLIProperties &cliProperties);

private:
    std::string m_description;
    std::string m_name;
    std::string m_version;
};

#endif  // SOURCE_OCTF_CLI_CLIPROPERTIES_H
