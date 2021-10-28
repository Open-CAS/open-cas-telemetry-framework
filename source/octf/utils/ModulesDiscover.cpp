/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/ModulesDiscover.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

void ModulesDiscover::getModulesList(NodesIdList &modules) {
    std::string dirPath = getFrameworkConfiguration().getUnixSocketDir();
    modules.clear();

    // Get a list of all sockets in directory for sockets
    std::list<std::string> sockets;
    fsutils::readDirectoryContents(dirPath, sockets, fsutils::FileType::Socket);

    for (const auto &socket : sockets) {
        // Check for read write permissions on file
        if (fsutils::checkPermissions(dirPath + "/" + socket,
                                      fsutils::PermissionType::ReadWrite)
            // Check if socket is not executable
            && !fsutils::checkPermissions(dirPath + "/" + socket,
                                          fsutils::PermissionType::Execute)) {
            try {
                // Finally create node ID
                modules.emplace_back(NodeId(socket));
            } catch (Exception &) {
            }
        }
    }
}

}  // namespace octf
