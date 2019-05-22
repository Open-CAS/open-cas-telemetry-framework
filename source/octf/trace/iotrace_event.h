/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
#define SOURCE_OCTF_TRACE_IOTRACE_EVENT_H

typedef uint64_t log_sid_t;

#define IOTRACE_EVENT_VERSION_MAJOR 1
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
} __attribute__((packed, aligned(8)));

#endif  // SOURCE_OCTF_TRACE_IOTRACE_EVENT_H
