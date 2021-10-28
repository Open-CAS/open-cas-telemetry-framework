/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/FileOperations.h>

#include <dirent.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

namespace octf {
namespace fsutils {

static constexpr int SUCCESS = 0;

/**
 * @brief Checks if given entry is of given type.
 * This serves as second check if readdir returns an entry of type unknown
 * (see readdir docs)
 * @param entry Directory entry to check
 * @param type Type of file
 *
 * @retval true Entry is of given type
 * @retval false Entry is not of given type
 */
static bool isType(struct dirent *entry, FileType type) {
    struct stat stbuf;
    if (stat(entry->d_name, &stbuf) == SUCCESS) {
        switch (type) {
        case FileType::Directory:
            return S_ISDIR(stbuf.st_mode);
        case FileType::Pipe:
            return S_ISFIFO(stbuf.st_mode);
        case FileType::Link:
            return S_ISLNK(stbuf.st_mode);
        case FileType::Regular:
            return S_ISREG(stbuf.st_mode);
        case FileType::Socket:
            return S_ISSOCK(stbuf.st_mode);
        case FileType::Any:
            return true;
        default:
            return false;
        }
    }
    // Error occurred
    return false;
}

/**
 * @brief Converts FileType to dirent.h file type
 * @param type File type to be converted
 * @return value of file type as dirent.h value
 */
static int getDirentType(FileType type) {
    switch (type) {
    case FileType::Directory:
        return DT_DIR;
    case FileType::Link:
        return DT_LNK;
    case FileType::Pipe:
        return DT_FIFO;
    case FileType::Regular:
        return DT_REG;
    case FileType::Socket:
        return DT_SOCK;
    case FileType::Any:
        return DT_UNKNOWN;
    default:
        return DT_UNKNOWN;
    }
}

/**
 * @brief Converts PermissionType to permissions as unistd.h value
 * @param permission Permission to be converted
 * @return Unistd.h value of this permission
 */
static int getUnistdPermissions(PermissionType permission) {
    switch (permission) {
    case PermissionType::Execute:
        return X_OK;
    case PermissionType::Read:
        return R_OK;
    case PermissionType::Write:
        return W_OK;
    case PermissionType::ReadWrite:
        return (R_OK | W_OK);
    default:
        // File exists
        return F_OK;
    }
}

/**
 * @brief Checks if file is not a filesystem link "." or ".."
 * @param entry File entry
 * @retval true File is not "." or ".."
 * @retval false File is "." or ".."
 */
static bool isRealFile(dirent *entry) {
    if (std::string(entry->d_name) == "." ||
        std::string(entry->d_name) == "..") {
        return false;
    }
    return true;
}

/**
 * @brief Removes directory
 * @param path Path to directory
 * @return Result of operation
 */
static bool removeDirectory(const std::string &path) {
    // To remove the directory first we have to delete all of its contents
    // recursively
    struct dirent *entry = nullptr;
    DIR *directory = nullptr;
    directory = opendir(path.c_str());

    if (directory == nullptr) {
        return false;
    }

    while ((entry = readdir(directory)) != NULL) {
        // Ommit "." and ".." files
        if (!isRealFile(entry)) {
            continue;
        }

        if (isType(entry, FileType::Directory)) {
            if (!removeDirectory(std::string(entry->d_name))) {
                return false;
            }
        } else {
            if (remove(std::string(path + "/" + entry->d_name).c_str()) !=
                SUCCESS) {
                return false;
            }
        }
    }

    // Remove empty directory
    if (rmdir(path.c_str()) != SUCCESS) {
        return false;
    }

    return true;
}

bool checkPermissions(std::string file, PermissionType permission) {
    // Check if there is specified permission
    int result = ::access(file.c_str(), getUnistdPermissions(permission));
    if (result != SUCCESS) {
        return false;
    }

    return true;
}

static bool _createDirectory(const std::string &childDirectory) {
    if ("" == childDirectory) {
        // Recurrence exit condition
        return true;
    }

    struct stat st;
    // If a directory exists, no need to go deeper, return success
    if (!stat(childDirectory.c_str(), &st) && S_ISDIR(st.st_mode)) {
        return true;
    }

    auto pos = childDirectory.rfind("/");
    if (pos != childDirectory.npos) {
        // If a directory doesn't exist, strip everything after last / from the
        // path and check parent directory
        bool result = _createDirectory(childDirectory.substr(0, pos));
        if (!result) {
            // Subdirectory creation error
            return false;
        }
    }

    // If parent check was successful, create the missing directory
    if (mkdir(childDirectory.c_str(), S_IRWXU | S_IRWXG)) {
        return false;
    }

    return true;
}

bool createDirectory(const std::string &path) {
    if ("" == path) {
        return false;
    }

    return _createDirectory(path);
}

bool readDirectoryContents(const std::string &path,
                           std::list<std::string> &files,
                           FileType type) {
    DIR *dir = opendir(path.c_str());
    struct dirent *entry;

    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            // Ommit "." and ".." files
            if (!isRealFile(entry)) {
                continue;
            }

            if (type == FileType::Any || entry->d_type == getDirentType(type)) {
                // Add file to list
                files.push_back(entry->d_name);

            } else if (entry->d_type == DT_UNKNOWN) {
                // Type unknown may be any type (see readdir docs)
                if (isType(entry, type)) {
                    files.push_back(entry->d_name);
                } else {
                    continue;
                }
            }
        }
        closedir(dir);

    } else {
        // Could not open directory
        return false;
    }

    return true;
}

bool readDirectoryContentsRecursive(const std::string &path,
                                    std::list<std::string> &files,
                                    FileType type) {
    std::list<std::string> directories;
    bool result = true;

    // Add this directory contents
    result = readDirectoryContents(path, files, type);

    // Get directories
    if (!readDirectoryContents(path, directories, FileType::Directory)) {
        result = false;
    }

    // Scan them recursively
    for (const auto &dir : directories) {
        std::list<std::string> nestedFiles;
        if (!readDirectoryContentsRecursive(path + "/" + dir, nestedFiles,
                                            type)) {
            result = false;
        }

        // Prepend current directory name
        for (const auto &nestedFile : nestedFiles) {
            files.push_back(dir + "/" + nestedFile);
        }
    }

    return result;
}

bool removeFile(const std::string &path) {
    // Check write permissions on file
    if (!checkPermissions(path, PermissionType::Write)) {
        return false;
    }

    // Attempt to remove as directory
    if (!removeDirectory(path)) {
        // Removing as directory failed, remove as normal file
        if (remove(path.c_str()) != SUCCESS) {
            return false;
        }
    }

    return true;
}

}  // namespace fsutils
}  // namespace octf
