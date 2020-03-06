/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_IFILESYSTEMVIEWER_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_IFILESYSTEMVIEWER_H

#include <string>

namespace octf {

struct InodeId {
    uint64_t inode;
    timespec cdate;

    InodeId()
            : inode(0)
            , cdate{} {}
    InodeId(uint64_t id, struct timespec cdate)
            : inode(id)
            , cdate(cdate) {}
    bool operator==(const InodeId other) const {
        return inode == other.inode && cdate.tv_sec == other.cdate.tv_sec &&
               cdate.tv_nsec == other.cdate.tv_nsec;
    }
};

/**
 * @ingroup Statistics
 * @interface File system viewer
 *
 * @brief Utility interface which allows to view file system structure
 *
 * This interface has to be provided to FilesystemStatistics to compute
 * filesystem statistics
 */
class IFileSystemViewer {
public:
    IFileSystemViewer() = default;
    virtual ~IFileSystemViewer() = default;

    /**
     * @brief Get Parent ID for specified file ID
     *
     * @param id File ID
     * @return Parent ID
     */
    virtual InodeId getParentId(InodeId id) const = 0;

    /**
     * @brief Gets base name of file
     *
     * If file respectively consists of:
     * - string of alphabetic chars, followed by
     * - string of non-alphabetic chars, followed by
     * - dot, followed by
     * - any file extension
     * Then file prefix is the first part: <b>string of alphabetic chars</b>
     *
     * | %File Name    | %File Prefix |
     * |---------------|--------------|
     * | foo.txt       | foo          |
     * | bar           | bar          |
     * | log1410       | log          |
     * | log1683.dat   | log          |
     * | db-1918.bin   | db           |
     * | db_1920:1410  | db           |
     *
     * @param id %File ID
     * @return %File name prefix
     */
    virtual std::string getFileNamePrefix(InodeId id) const = 0;

    /**
     * @brief Gets file name of specified file ID
     *
     * @param id File ID
     * @return File name
     */
    virtual std::string getFileName(InodeId id) const = 0;

    /**
     * @brief Gets file extension of specified file ID
     *
     * @param id File ID
     * @return File extension
     */
    virtual std::string getFileExtension(InodeId id) const = 0;

    /**
     * @brief Gets file path of specified file ID on a file system
     *
     * @param id File ID
     * @return File path
     */
    virtual std::string getFilePath(InodeId id) const = 0;

    /**
     * @brief Gets FS directory path of specified file ID
     *
     * @param id File Id
     * @return Directory path
     */
    virtual std::string getDirPath(InodeId id) const = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_IFILESYSTEMVIEWER_H
