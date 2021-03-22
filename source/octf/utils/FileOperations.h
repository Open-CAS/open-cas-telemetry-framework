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
 * @brief Checks given file for permissions using real user id
 *
 * @note When using this function make sure to avoid TOCTOU vulnerability
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
 * @note When using this function make sure to avoid TOCTOU vulnerability
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

/**
 * @brief Checks if file exist for specified path
 *
 * Checks if the file exists and optionally is of the specified type
 *
 * @param path Path of file/directory to check if exist
 * @param type Type of file
 *
 * @retval true File exists and its type is like specified
 * @retval false File does not exist or its type isn't like specified
 */
bool exist(const std::string &path, FileType type = FileType::Any);

/**
 * @brief Writes data to the file
 *
 * @param path Path to the file into which write data
 * @param data data which will be written to the file
 *
 * @retval true Data written to the file successfully
 * @retval false An error occurred when writing data to the file
 */
bool write(const std::string &path, const std::string &data);

/**
 * @brief Reads data from the file
 *
 * @param path Path to the file from which read data
 * @param[out] data Read data
 *
 * @retval true Data read from the file successfully
 * @retval false An error occurred when reading data from the file
 */
bool read(const std::string &path, std::string &data);

}  // namespace fsutils
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_FILEOPERATIONS_H
