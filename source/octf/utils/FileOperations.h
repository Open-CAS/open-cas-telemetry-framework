/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_FILEOPERATIONS_H
#define SOURCE_OCTF_UTILS_FILEOPERATIONS_H

#include <list>
#include <string>

namespace octf {
namespace fsutils {

enum class PermissionType { Read, Write, ReadWrite, Execute };

enum class FileType { Regular, Directory, Link, Pipe, Socket, Any };

/**
 * @brief Creates a directory at specified path recursively
 *
 * The mode applied when creating dir(s) is S_IRWXU | S_IRWXG.
 *
 * @param path Directory path to be created
 *
 * @retval true - Dir(s) created successfully
 * @retval false - operation failed
 */
bool createDirectory(const std::string &path);

/**
 * @brief Lists files in specified directory,
 *  and adds them to a list of strings with path relative to given directory.
 *  Type argument specifies file type to be included (default: regular file)
 *
 * @note '.' and ".." files are ignored
 *
 * @param [in] path Directory to be read for files
 * @param [in] type Type of files to be added to the list of files
 * @param [out] files List of paths to read files
 *
 * @retval true  Directory successfully read
 * @retval false Could not read directory
 */
bool readDirectoryContents(const std::string &path,
                           std::list<std::string> &files,
                           FileType type);

/**
 * @brief Reads specified directory recursively, and appends the contents to
 *  'files' list as strings with path relative to given directory.
 *  Type argument specifies file type to be included (default: regular file)
 *
 * @note '.' and ".." files are ignored
 *
 * @param [in] path Directory to be read for files
 * @param [in] type Type of files to be added to the list of files
 * @param [out] files List of paths to read files
 *
 * @retval true  Directory successfully read
 * @retval false Some directories were not read,
 *  possibly due to a lack of permissions
 */
bool readDirectoryContentsRecursive(const std::string &path,
                                    std::list<std::string> &files,
                                    FileType type);

/**
 * @brief Checks given file for permissions
 *
 * @param file Absolute path to the file
 * @param permission Permission to check
 *
 * @retval true file is accessible
 * @retval false file is not accessible
 */
bool checkPermissions(std::string file, PermissionType permission);

/**
 * @brief Removes specified file. If it's a directory, it is removed with its
 * contents.
 *
 * @note In case of failure of removal of any of the contained files directory
 * contents may be partially removed.
 *
 * @param path Path to file
 *
 * @retval true Successfully removed file
 * @retval false Could not remove file
 */
bool removeFile(const std::string &path);

}  // namespace fsutils
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_FILEOPERATIONS_H
