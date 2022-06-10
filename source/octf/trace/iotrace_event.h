/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
#define SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__KERNEL__)
#include <linux/types.h>
#elif defined(__BPF__)
#else
#include <stdint.h>
#include <time.h>
#define timespec64 timespec
#endif

typedef uint64_t log_sid_t;

#define IOTRACE_EVENT_VERSION_MAJOR 6
#define IOTRACE_EVENT_VERSION_MINOR 0

#define IOTRACE_MAGIC 0x5a8e454ace7a51c1ULL

/**
 * @brief iotrace (event) type
 */
typedef enum {
    /** Device description, providing mapping between device id and path for
     * subsequent traces */
    iotrace_event_type_device_desc,

    /** IO queue event */
    iotrace_event_type_io,

    /** IO completion event */
    iotrace_event_type_io_cmpl,

    /** Filesystem meta information event */
    iotrace_event_type_fs_meta,

    /** File name event */
    iotrace_event_type_fs_file_name,

    /** Filesystem file created, remove, etc event */
    iotrace_event_type_fs_file_event
} iotrace_event_type;

/**
 * @brief Generic iotrace event
 */
struct iotrace_event_hdr {
    /** Event sequence ID */
    log_sid_t sid;

    /** Time stamp */
    uint64_t timestamp;

    /** Trace event type, iotrace_event_type enunerator */
    uint32_t type;

    /** Size of this event, including header */
    uint32_t size;
} __attribute__((packed, aligned(8)));

static inline void iotrace_event_init_hdr(struct iotrace_event_hdr *hdr,
                                          iotrace_event_type type,
                                          uint64_t sid,
                                          uint64_t timestamp,
                                          uint32_t size) {
    hdr->sid = sid;
    hdr->timestamp = timestamp;
    hdr->type = type;
    hdr->size = size;
}

/**
 *  @brief Device description event
 */
struct iotrace_event_device_desc {
    /** Event header */
    struct iotrace_event_hdr hdr;

    /** Device Id */
    uint64_t id;

    /** Device size in sectors */
    uint64_t device_size;

    /** Canonical device name */
    char device_name[256];

    /** Device model */
    char device_model[256];
} __attribute__((packed, aligned(8)));

/** @brief IO operation type */
typedef enum {
    /** Read */
    iotrace_event_operation_rd = 'R',

    /** Write */
    iotrace_event_operation_wr = 'W',

    /** Discard */
    iotrace_event_operation_discard = 'D',
} iotrace_event_operation_t;

/** @brief IO operation type */
typedef enum {
    /** Flush */
    iotrace_event_flag_flush = 1 << 0,
    /** FUA */
    iotrace_event_flag_fua = 1 << 1,
    /** Metadata IO */
    iotrace_event_flag_metadata = 1 << 2,
    /** Direct IO */
    iotrace_event_flag_direct = 1 << 3,
    /** Read ahead IO */
    iotrace_event_flag_readahead = 1 << 4
} iotrace_event_flag_t;

/**
 * @brief IO trace event
 */
struct iotrace_event {
    /** Trace event header */
    struct iotrace_event_hdr hdr;
    /**
     * @brief IO ID
     *
     * This ID can be used by the tracing environment to assign an ID to the IO.
     *
     * @note Zero means not set.
     */
    uint64_t id;

    /** Address of IO in sectors */
    uint64_t lba;

    /** Size of IO in sectors */
    uint32_t len;

    /** Device ID */
    uint32_t dev_id;

    /** Operation flags: flush, fua, ... .
     * Values according to iotrace_event_flag_t enum
     * are summed (OR-ed) together. */
    uint32_t flags;

    /** Operation type: read, write, discard
     * (iotrace_event_operation_t enumerator) **/
    uint8_t operation;

    /** Write hint associated with IO */
    uint8_t write_hint;
} __attribute__((packed, aligned(8)));

/**
 * @brief IO completion trace event
 */
struct iotrace_event_completion {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /**
     * @brief The ID of an IO which is completed
     *
     * Environment can set ID of IO which is completed
     */
    uint64_t ref_id;

    /** Address of completed IO in sectors */
    uint64_t lba;

    /** Size of completed IO in sectors */
    uint32_t len;

    /**
     * Result of completed IO
     *
     * Value equals zero means no errors and IO was completed successfully.
     * Otherwise IO ended with an error.
     */
    int32_t error;

    /** Device ID */
    uint32_t dev_id;
} __attribute__((packed, aligned(8)));

/*/
 * @brief Structure identifying a given file
 */
struct iotrace_event_file_id {
    /** File ID */
    uint64_t id;

    /**
     * inode creation date stored in entry
     */
    struct timespec64 ctime;
} __attribute__((packed, aligned(8)));

/**
 * @brief IO trace event metadata (e.g. filesystem meta information)
 *
 * If an application works on top of a filesystem it may perform IO operations
 * to files. This structure contains meta information about IO in domain of
 * filesystem. For example it provides information about file size,
 * file offset of IO, etc.
 */
struct iotrace_event_fs_meta {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /**
     * Event reference ID reference to IO trace event associated with this event
     */
    log_sid_t ref_id;

    /** File ID */
    struct iotrace_event_file_id file_id;

    /** File offset in sectors */
    uint64_t file_offset;

    /** File size in sectors */
    uint64_t file_size;

    /** ID of the partition the file belongs to */
    uint64_t partition_id;
} __attribute__((packed, aligned(8)));

struct iotrace_event_fs_file_name {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /** ID of the partition the file belongs to */
    uint64_t partition_id;

    /** File ID */
    struct iotrace_event_file_id file_id;

    /** File parent ID */
    struct iotrace_event_file_id file_parent_id;

    /** File name */
    char file_name[64];
    // TODO (mariuszbarczak) Make file name size dynamic
} __attribute__((packed, aligned(8)));

/**
 * @brief Type of filesystem file event
 */
typedef enum {
    /** File created */
    iotrace_fs_event_create = 0,

    /** File deleted */
    iotrace_fs_event_delete,

    /** File moved to */
    iotrace_fs_event_move_to,

    /** File moved from */
    iotrace_fs_event_move_from
} iotrace_fs_event_type;

/**
 * @brief Event relating to filesystem event such as create, delete, etc.
 *
 * Events are created only when a directory with given file is 'marked'.
 * Directories are 'marked' upon opening them from another 'marked' directory,
 * or when bio is sent to file in given directory.
 * Note that until first bio is sent, no events are created, as no directory is
 * marked.
 */
struct iotrace_event_fs_file_event {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /** ID of the partition the file belongs to */
    uint64_t partition_id;

    /** File ID */
    struct iotrace_event_file_id file_id;

    /** Event type */
    iotrace_fs_event_type fs_event_type;
} __attribute__((packed, aligned(8)));

#ifdef __cplusplus
}
#endif
#endif  // SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
