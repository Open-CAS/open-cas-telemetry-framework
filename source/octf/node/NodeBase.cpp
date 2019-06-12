/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/node/NodeBase.h>

#include <algorithm>
#include <memory>
#include <octf/interface/InterfaceCliImpl.h>
#include <octf/interface/InterfaceIdentificationImpl.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

NodeBase::NodeBase(const NodeId &id)
        : NodeGeneric(id)
        , m_config(nullptr) {}

bool NodeBase::initCommon() {
    // Call common init of parent class
    if (!NodeGeneric::initCommon()) {
        return false;
    }

    if (!createInterface<InterfaceIdentificationImpl>(this)) {
        throw Exception("Error creating identification interface.");
    }

    return true;
}

void NodeBase::deinitCommon() {
    writeSettings();

    // Call common deinit of parent class
    NodeGeneric::deinitCommon();
}

bool NodeBase::readSettings() {
    if (m_config) {
        try {
            ProtobufReaderWriter rw(
                    getFrameworkConfiguration().getNodeSettingsFilePath(
                            getNodePath()));

            if (rw.read(*m_config)) {
                return true;
            } else {
                log::cerr << "Cannot read configuration for file "
                          << rw.getFilePath() << std::endl;

                return false;
            }

        } catch (Exception& ex) {
            log::cerr << ex.getMessage() << std::endl;
            throw Exception("Could not read node settings.");
        }
    }

    return false;
}

bool NodeBase::writeSettings() {
    if (m_config) {
        try {
            ProtobufReaderWriter rw(
                    getFrameworkConfiguration().getNodeSettingsFilePath(
                            getNodePath()));

            if (rw.write(*m_config)) {
                return true;
            } else {
                log::cerr << "Cannot write configuration for file "
                          << rw.getFilePath() << std::endl;

                return false;
            }
        } catch (Exception& ex) {
            log::cerr << ex.getMessage() << std::endl;
            return false;
        }
    }

    return false;
}

bool NodeBase::removeSettings() {
    if (m_config) {
        try {
            ProtobufReaderWriter rw(
                    getFrameworkConfiguration().getNodeSettingsFilePath(
                            getNodePath()));

            if (rw.isFileAvailable()) {
                if (rw.remove()) {
                    return true;
                }

                log::cerr << "Cannot remove configuration for file "
                          << rw.getFilePath() << std::endl;

                return false;
            }
        } catch (Exception& ex) {
            log::cerr << ex.getMessage() << std::endl;
            return false;
        }
    }

    return true;
}

bool NodeBase::areSettingsAvailable() {
    if (m_config) {
        try {
            ProtobufReaderWriter rw(
                    getFrameworkConfiguration().getNodeSettingsFilePath(
                            getNodePath()));

            return rw.isFileAvailable();

        } catch (Exception& ex) {
            log::cerr << ex.getMessage() << std::endl;
            return false;
        }
    }

    return false;
}

}  // namespace octf
