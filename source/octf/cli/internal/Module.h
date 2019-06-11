/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_MODULE_H
#define SOURCE_OCTF_CLI_INTERNAL_MODULE_H

#include <iomanip>
#include <string>

namespace octf {
namespace cli {

/**
 * @brief Class describing module
 */
class Module {
public:
    /**
     * @brief Default constructor
     */
    Module();

    virtual ~Module() = default;

    /**
     * @return Long key of module
     */
    const std::string &getLongKey() const;

    /**
     * @param longKey Long key of module
     */
    void setLongKey(const std::string &longKey);

    /**
     * @return Short key of module
     */
    const std::string &getShortKey() const;

    /**
     * @param shortKey Short key of module
     */
    void setShortKey(const std::string &shortKey);

    /**
     * @brief Assignment operator
     * @param other Module to take data from
     * @return Module with copied data
     */
    virtual Module &operator=(const Module &other);

    /**
     * @return Module description
     */
    const std::string &getDesc() const;

    /**
     * @param desc Module description
     */
    void setDesc(const std::string &desc);

    /**
     * @return Is this module local
     */
    bool isLocal() const;

    /**
     * @param isLocal Set this module as local or remote
     */
    void setLocal(bool isLocal);

private:
    std::string m_shortKey;
    std::string m_longKey;
    std::string m_desc;
    bool m_isLocal;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_MODULE_H
