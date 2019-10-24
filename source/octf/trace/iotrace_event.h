/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
#define SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

typedef uint64_t log_sid_t;

#define IOTRACE_EVENT_VERSION_MAJOR 2
#define IOTRACE_EVENT_VERSION_MINOR 0

#define IOTRACE_MAGIC 0x5a8e454ace7a51c1ULL

/**
 * @brief iotrace (event) type
 */
typedef enum {
    /** Device description, providing mapping between device id and path for
     * subsequent traces */
    iotrace_event_type_device_desc = 0x00,

    /** IO queue event */
    iotrace_event_type_io = 'Q',

    /** IO completion event */
    iotrace_event_type_io_cmpl = 'C',

    /** Filesystem meta information event */
    iotrace_event_type_fs_meta = 'M',

    /** File name event */
    iotrace_event_type_fs_file_name = 'N',

    /** Filesystem file created, remove, etc event */
    iotrace_event_type_fs_file_event = 'F'
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
    char device_name[32];
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
} iotrace_event_flag_t;

/**
 * @brief IO trace event
 */
struct iotrace_event {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /** Address of IO in sectors */
    uint64_t lba;

    /** Size of IO in sectors */
    uint32_t len;

    /** IO class of IO */
    uint32_t io_class;

    /** Device ID */
    uint32_t dev_id;

    /** Operation type: read, write, discard
     * (iotrace_event_operation_t enumerator) **/
    uint32_t operation;

    /** Operation flags: flush, fua, ... .
     * Values according to iotrace_event_flag_t enum
     * are summed (OR-ed) together. */
    uint32_t flags;

    /** Write hint associated with IO */
    uint8_t write_hint;
} __attribute__((packed, aligned(8)));

/**
 * @brief IO completion trace event
 */
struct iotrace_event_completion {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

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
     * Event sequence ID reference to IO trace event associated with this event
     */
    log_sid_t ref_sid;

    /** File ID */
    uint64_t file_id;

    /** File offset in sectors */
    uint64_t file_offset;

    /** File size in sectors */
    uint64_t file_size;
} __attribute__((packed, aligned(8)));

struct iotrace_event_fs_file_name {
    /** Trace event header */
    struct iotrace_event_hdr hdr;

    /** Device Id */
    uint64_t device_id;

    /** File ID */
    uint64_t file_id;

    /** File parent ID */
    uint64_t file_parent_id;

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

    /** Device Id */
    uint64_t device_id;

    /** Event type */
    iotrace_fs_event_type fs_event_type;

    /** File ID */
    uint64_t file_id;

    /** Parent directory ID */
    uint64_t parent_id;

} __attribute__((packed, aligned(8)));

#ifdef __cplusplus
}
#endif
#endif  // SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
