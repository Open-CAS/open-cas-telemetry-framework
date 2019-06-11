/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CLIPROPERTIES_H
#define SOURCE_OCTF_CLI_CLIPROPERTIES_H

#include <string>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace cli {

/**
 * @brief Class managing properties of CLI
 */
class CLIProperties {
public:
    CLIProperties() = default;
    virtual ~CLIProperties() = default;
    CLIProperties(CLIProperties const &) = default;
    CLIProperties &operator=(CLIProperties const &) = default;

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
    std::string m_description;
    std::string m_name;
    std::string m_version;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_CLIPROPERTIES_H
