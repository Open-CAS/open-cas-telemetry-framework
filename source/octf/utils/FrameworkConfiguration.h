/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_FRAMEWORKCONFIGURATION_H
#define SOURCE_OCTF_UTILS_FRAMEWORKCONFIGURATION_H

#include <string>
#include <octf/node/NodeId.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Utility class for getting framework configuration
 */
class FrameworkConfiguration : NonCopyable {
public:
    /**
     * @brief Gets OCTF framework version
     *
     * @return OCTF framework version
     */
    const std::string &getVersion() const;

    /**
     * @brief Gets UNIX socket dir location
     *
     * @return UNIX socket dir location
     */
    const std::string &getUnixSocketDir() const;

    /**
     * @brief Gets trace dir location
     *
     * @return Trace dir location
     */
    const std::string &getTraceDir() const;

    /**
     * @brief Gets UNIX socket file location for specified node
     *
     * @note For nodes tree, socket is opened for root node. Thus
     * input parameter shall be the root node ID.
     *
     * @param id Node id
     *
     * @return UNIX socket file location
     */
    std::string getUnixSocketFilePath(const NodeId &id) const;

    /**
     * @brief Gets node configuration dir location
     *
     * @return UNIX socket dir location
     */
    const std::string &getNodeSettingsDir() const;

    /**
     * @brief Gets configuration file location for specified node
     *
     * @param path Node path
     *
     * @return Settings file location
     */
    std::string getNodeSettingsFilePath(const NodePath &path) const;

    /**
     * @brief Gets trace directory location for specified node
     *
     * @param id Node id
     *
     * @return Trace directory location
     */
    std::string getNodeTraceDirectoryPath(const NodePath &path) const;

    /**
     * @brief Gets file basename for specified node path
     *
     * Path consists of node IDs (it's vector of node IDs). The format of
     * basename is:
     * path[0]:path[1]:...:path[path.size()-1]
     *
     * @param path Node path
     *
     * @return file basename of node path
     */
    std::string getNodePathBasename(const NodePath &path) const;

    virtual ~FrameworkConfiguration() = default;

private:
    FrameworkConfiguration();
    friend const FrameworkConfiguration &getFrameworkConfiguration();
};

/**
 * @brief Gets framework configuration
 *
 * @return framework configuration
 */
const FrameworkConfiguration &getFrameworkConfiguration();

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_FRAMEWORKCONFIGURATION_H
