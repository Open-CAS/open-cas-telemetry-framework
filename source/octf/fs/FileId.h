/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_FS_FILEID_H
#define SOURCE_OCTF_FS_FILEID_H

#include <octf/proto/parsedTrace.pb.h>
#include <octf/proto/trace.pb.h>

namespace octf {

struct FileId {
    uint64_t partitionId;
    uint64_t id;
    timespec creationDate;

    FileId()
            : partitionId(0)
            , id(0)
            , creationDate{0, 0} {}

    FileId(uint64_t partId, uint64_t fileId, timespec cdate)
            : partitionId(partId)
            , id(fileId)
            , creationDate(cdate) {}

    FileId(const proto::trace::FileId &fileid)
            : partitionId(fileid.partitionid())
            , id(fileid.id()) {
        creationDate.tv_sec = fileid.creationdate().seconds();
        creationDate.tv_nsec = fileid.creationdate().nanos();
    }

    FileId(const proto::trace::EventIoFilesystemFileName &event)
            : FileId(event.fileid()) {}

    FileId(const proto::trace::EventIoFilesystemMeta &event)
            : FileId(event.fileid()) {}

    FileId(const proto::trace::EventIoFilesystemFileEvent &event)
            : FileId(event.fileid()) {}

    FileId(const proto::trace::ParsedEvent &event)
       : partitionId(event.device().partition())
       , id(event.file().id())
       {
        creationDate.tv_sec = event.file().creationdate().seconds();
        creationDate.tv_nsec = event.file().creationdate().nanos();
       }
    FileId(const FileId &other)
            : partitionId(other.partitionId)
            , id(other.id)
            , creationDate(other.creationDate) {}

    FileId &operator=(const FileId &other) {
        if (this != &other) {
            partitionId = other.partitionId;
            id = other.id;
            creationDate = other.creationDate;
        }

        return *this;
    }

    bool operator==(const FileId &other) const {
        return id == other.id && partitionId == other.partitionId &&
               creationDate.tv_sec == other.creationDate.tv_sec &&
               creationDate.tv_nsec == other.creationDate.tv_nsec;
    }

    bool operator!=(const FileId &other) const {
        return !(*this == other);
    }

    bool operator<(const FileId &other) const {
        if (partitionId != other.partitionId) {
            return partitionId < other.partitionId;
        } else if (id != other.id) {
            return id < other.id;
        } else if (creationDate.tv_sec != other.creationDate.tv_sec) {
            return creationDate.tv_sec < other.creationDate.tv_sec;
        } else {
            return creationDate.tv_nsec < other.creationDate.tv_nsec;
        }
    }
};

}

#endif /* SOURCE_OCTF_FS_FILEID_H */
