/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_NODE_NODEBASE_H
#define SOURCE_OCTF_NODE_NODEBASE_H

#include <memory>
#include <octf/node/NodeGeneric.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Shared reference of node settings
 *
 * It has to be protocol buffer message. The framework uses protocol buffer's
 * ability of parsing and serialization the message during reading and writing
 * settings.
 */
typedef std::shared_ptr<google::protobuf::Message> NodeSettingsShRef;

/**
 * @brief Base class for normal nodes (not shadow ones).
 */
class NodeBase : public NodeGeneric {
public:
    NodeBase(const NodeId &id);
    virtual ~NodeBase() = default;

    bool initCommon() override;

    void deinitCommon() override;

    /**
     * @brief Creates and adds an interface to the list.
     *
     * @typedef T Class type of interface to be created and added
     * @typedef Args Parameters of Class type T's constructor
     *
     * @return Shared pointer to the interface
     *
     * @retval Valid shared pointer indicates success of operation
     *
     * @retval null shared pointer indicates operation failure
     * (because of some error or already existing interface with the same
     * InterfaceId on the list)
     */
    template <typename T, typename... Args>
    std::shared_ptr<T> createInterface(Args... args) {
        try {
            std::shared_ptr<T> interface = std::make_shared<T>(args...);

            if (addInterface(interface)) {
                return interface;
            } else {
                // Find interface, if already exists return it
                return findInterface<T>();
            }
        } catch (Exception &) {
            return nullptr;
        }
    }

    /**
     * @brief Initializes settings for this node,
     *
     * @note If settings had been written before then tries read it
     *
     * @typedef T Node settings type
     *
     * @retval true Settings created and read successfully
     * @retval false Error while reading settings
     */
    template <typename T>
    bool initSettings() {
        m_settings = std::make_shared<T>();

        if (areSettingsAvailable()) {
            return readSettings();
        }

        return true;
    }

    /**
     * @brief Gets reference of node settings
     *
     * @typedef T Node settings type
     *
     * @return reference of node settings
     */
    template <typename T>
    std::shared_ptr<T> getSettings() {
        return std::dynamic_pointer_cast<T>(m_settings);
    }

    /**
     * @brief Check if settings are available for this node
     *
     * Settings are available if previously they had been written into
     * persistent storage.
     *
     * @retval true Settings are available
     * @retval false Settings are not available
     */
    bool areSettingsAvailable();

    /**
     * @brief Reads node settings
     *
     * Read settings from persistent storage if it had been written before
     *
     * @return operation result
     */
    bool readSettings();

    /**
     * @brief Writes node settings
     *
     * During deinitialization settings can be written into persistent
     * storage. And then during restart it can be read.
     *
     * @return operation result
     */
    bool writeSettings();

    /**
     * @brief Removes node settings persistently
     *
     * If nodes is going to be obsoleted and not used any more the settings
     * shall be removed from persistent storage
     *
     * @note if settings not found then operation result is successful
     *
     * @return operation result
     */
    bool removeSettings();

private:
    /**
     * Node settings
     */
    NodeSettingsShRef m_settings;
};

}  // namespace octf

#endif  // SOURCE_OCTF_NODE_NODEBASE_H
