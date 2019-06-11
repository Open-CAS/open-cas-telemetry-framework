/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_CMD_COMMAND_H
#define SOURCE_OCTF_CLI_CMD_COMMAND_H
#include <map>
#include <string>
#include <octf/cli/internal/cmd/ICommand.h>
#include <octf/cli/internal/param/IParameter.h>

namespace octf {

class CLIList;

/**
 * @brief Generic implementation of IParameter interface
 */
class Command : public ICommand {
public:
    /**
     * @brief Empty command constructor
     */
    Command();

    virtual ~Command();

    const std::string &getShortKey() const override;

    const std::string &getLongKey() const override;

    const std::string &getDesc() const override;

    uint32_t getParamsCount() const override;

    virtual std::shared_ptr<IParameter> getParam(
            const std::string &name) override;

    virtual std::shared_ptr<IParameter> getParamByIndex(
            const int32_t index) override;

    void parseParamValues(CLIList &cliList) override;

    void getHelp(std::stringstream &ss) const override;

    void getCommandUsage(std::stringstream &ss) const override;

    bool isHidden() const override;

    bool isLocal() const override;

    void checkParamMissing() const override;

    void setShortKey(const std::string &key) override;

    void setLongKey(const std::string &key) override;

    void setDesc(const std::string &desc) override;

    void setHidden(bool hidden) override;

    void setLocal(bool local) override;

    void addParam(std::shared_ptr<IParameter> param) override;

private:
    std::string m_shortKey;
    std::string m_longKey;
    std::string m_desc;
    std::map<std::string, std::shared_ptr<IParameter>> m_params;
    bool m_hidden;
    bool m_local;
};

}  // namespace octf

#endif  // SOURCE_OCTF_CLI_CMD_COMMAND_H
