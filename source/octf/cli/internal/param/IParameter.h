/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_CLI_INTERNAL_PARAM_IPARAMETER_H
#define SOURCE_OCTF_CLI_INTERNAL_PARAM_IPARAMETER_H

#include <string>
#include <octf/proto/InterfaceCLI.pb.h>

namespace octf {
namespace cli {

class CLIElement;

/**
 * @brief CLI parameter interface
 *
 * CLI parameter has a name (long name) e.g. "--parameter"
 * and a key (short name) e.g. "-p". Parameter can be
 * 'multiple value' meaning it can parse multiple delimited values
 * from input and add them as separate values to protobuf message.
 */
class IParameter {
public:
    /**
     * @brief Destructor
     */
    virtual ~IParameter() = default;

    /**
     * @brief Gets help for this parameter
     */
    virtual void getHelp(std::stringstream &ss) const = 0;

    /**
     * @return Is this parameter required
     */
    virtual bool isRequired() const = 0;

    /**
     * @return Does this parameter take multiple values
     *
     * @note If parameter does not support 'multiple value' this should
     * return false.
     */
    virtual bool isMultipleValue() const = 0;

    /**
     * @return Has this parameter been set (passed)
     */
    virtual bool isValueSet() const = 0;

    /**
     * @return The key (short name) of parameter
     */
    virtual const std::string &getShortKey() const = 0;

    /**
     * @return The long key of parameter
     */
    virtual const std::string &getLongKey() const = 0;

    /**
     * @return The description of parameter
     */
    virtual const std::string &getDesc() const = 0;

    /**
     * @return The index of parameter
     */
    virtual const int32_t &getIndex() const = 0;
    /**
     * @return If this parameter has a default value
     */
    virtual bool hasDefaultValue() const = 0;

    /**
     * @return Is this parameter hidden (doesn't show up in help)
     */
    virtual bool isHidden() const = 0;

    /**
     * @return Sets if the parameter is hidden (doesn't show up in help)
     */
    virtual void setHidden(bool hidden) = 0;

    /**
     * @retval True if parameter takes any value.
     * @retval False if parameter is a flag only (present or absent in CLI)
     */
    virtual bool hasValue() const = 0;

    /**
     * @param value Unparsed value of parameter
     * @note Upon successful execution of this method
     * isValueSet() should return true
     *
     * @note Parameters which take multiple values should
     * set multiple values delimited by PARAMETER_VALUE_DELIMITER
     */
    virtual void setValue(CLIElement value) = 0;

    /**
     * @param required Is this parameter required
     */
    virtual void setRequired(bool required) = 0;

    /**
     * @param multipleValue Does this parameter take multiple values
     */
    virtual void setMultipleValue(bool multipleValue) = 0;

    /**
     * @param desc The description of parameter
     */
    virtual void setDesc(const std::string &desc) = 0;

    /**
     * @param index The index of parameter
     */
    virtual void setIndex(const int32_t index) = 0;

    /**
     * @param what Possible values of parameter to be
     * shown in help
     */
    virtual void setWhat(const std::string &what) = 0;

    /**
     * @return Possible values of parameter to be
     * shown in help
     */
    virtual const std::string &getWhat() const = 0;

    /**
     * @param key The short key of parameter
     */
    virtual void setShortKey(const std::string &key) = 0;

    /**
     * @param name Long key of the parameter
     */
    virtual void setLongKey(const std::string &name) = 0;

    /**
     * @brief Setup parameter (parameter's options on the basis of protocol
     * buffer options)
     *
     * @param param Object holding definition of CLI parameter
     * specifying options (like short key, long key, default, etc...)
     */
    virtual void setOptions(const proto::CliParameter &param) = 0;

    /**
     * @param Parses value to protocol buffer message
     */
    virtual void parseToProtobuf(
            google::protobuf::Message *message,
            const google::protobuf::FieldDescriptor *fieldDescriptor) = 0;

protected:
    /**
     * @brief Mark this parameter as set
     */
    virtual void setValueSet() = 0;
};

}  // namespace cli
}  // namespace octf

#endif  // SOURCE_OCTF_CLI_INTERNAL_PARAM_IPARAMETER_H
