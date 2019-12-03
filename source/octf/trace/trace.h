/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_TRACE_H
#define SOURCE_OCTF_TRACE_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdbool.h>
#include <stdint.h>
#endif

/**
 * Defines minimum memory buffer size required to create trace
 */
#define OCTF_TRACE_MIN_BUFFER_SIZE (32ULL * 1024ULL)

/**
 * @typedef Handle of trace
 */
typedef struct octf_trace *octf_trace_t;

typedef void *octf_trace_event_handle_t;

#define OCTF_TRACE_HDR_SIZE 4096ULL
typedef struct {
    char hdr[OCTF_TRACE_HDR_SIZE];
} __attribute__((packed, aligned(8))) octf_trace_hdr_t;

/**
 * Trace open mode
 */
typedef enum {
    /**
     * Opening tracing in producer mode
     */
    octf_trace_open_mode_producer,

    /**
     * Opening tracing in consumer mode
     */
    octf_trace_open_mode_consumer,

} octf_trace_open_mode_t;

/**
 * @brief Checks if trace is empty
 *
 * @param trace Trace handle
 *
 * @retval 0 trace not empty
 * @retval 1 trace empty
 * @retval -EINVAL Trace is invalid
 *
 */
int octf_trace_is_empty(octf_trace_t trace);

/**
 * @brief Checks if trace is closed
 *
 * @param trace Trace handle
 *
 * @retval 0 trace not closed
 * @retval 1 trace closed
 * @retval -EINVAL Trace is invalid
 *
 */
int octf_trace_is_closed(octf_trace_t trace);

/**
 * @brief Opens trace for specified memory buffer
 *
 * Trace implementation based on circular buffer design pattern. Producer will
 * be able to push trace events with different sizes. Trace event can be read by
 * consumer of trace.
 *
 * Trace may be opened in two modes:
 * - producer
 * - consumer
 * First one is able to write into trace only. Second one may read from trace
 * only.
 *
 * This trace library is thread safe. However due to performance reason, single
 * producer and single consumer is recommended.
 *
 * @param[in] mempool Memory pool (buffer) for trace where events will be stored
 * @param[in] size Size of memory buffer
 * @param[in] hdr Optional trace header of size OCTF_TRACE_HDR_SIZE, not NULL
 * if header is allocated from separate memory pool
 * @param[in] mode Trace opening mode
 * @param[out] trace Trace handle pointer
 *
 * @retval 0 Success
 * @retval Non-zero Error occurred
 */
int octf_trace_open(void *mempool,
                    uint32_t size,
                    octf_trace_hdr_t *hdr,
                    octf_trace_open_mode_t mode,
                    octf_trace_t *trace);

/**
 * @brief Close trace
 *
 * @param[in,out] trace trace handle
 */
void octf_trace_close(octf_trace_t *trace);

/**
 * @brief Gets number of lost events
 *
 * @param trace Trace handle
 *
 * @retval 0 - No lost events
 * @retval Greater than 0 - Number of lost evenets
 * @retval -EINVAL - Trace is invalid
 */
int64_t octf_trace_get_lost_count(octf_trace_t trace);

/**
 * @brief Pushes event to the trace
 *
 * @note Only producer may push events to the trace.
 *
 * @param[in] trace Trace handle
 * @param[in] event Pointer at event to be written to the trace
 * @param[in] size Event size
 *
 * @retval 0 Event stored successfully
 * @retval -EINVAL Trace is invalid
 * @retval -EPERM Invalid open mode, only producer can push events
 * @retval -ENOSPC Event lost because no space in memory pool to store event
 */
int octf_trace_push(octf_trace_t trace, const void *event, const uint32_t size);

/**
 * @brief Allocates event buffer
 *
 * @note Only producer may allocate event buffer.
 *
 * @param[in] trace Trace handle
 * @param[out] ev_hndl Handle to allocated event
 * @param[out] event Pointer to allocated event
 * @param[in] size Event size to allocate
 *
 * @retval 0 Event allocated successfully
 * @retval -EINVAL Trace is invalid
 * @retval -EPERM Invalid open mode, only producer can allocate write events
 * @retval -ENOSPC Event lost because no space in memory pool to allocate event
 */
int octf_trace_get_wr_buffer(octf_trace_t trace,
                             octf_trace_event_handle_t *ev_hndl,
                             void **event,
                             const uint32_t size);

/**
 * @brief Marks event as written
 *
 * @note Only producer may mark the event.
 *
 * @param[in] trace Trace handle
 * @param[in] ev_hndl Handle to event
 *
 * @retval 0 Event commited successfully
 * @retval -EINVAL Trace or handle is invalid
 */
int octf_trace_commit_wr_buffer(octf_trace_t trace, octf_trace_event_handle_t ev_hndl);

/**
 * @brief Pops event from the trace
 *
 * @param trace[in] Trace handle
 * @param event[out] Buffer where event will be copied from trace
 * @param size[in, out] For input value, it is maximum size of buffer. After
 * successful read, value indicates size of read event
 *
 * @retval 0 Event read successfully
 * @retval -EINVAL Trace is invalid
 * @retval -EPERM Invalid open mode, only consumer can pop events
 * @retval -EBUSY Pop operation already in progress
 * @retval -EBADF Trace empty and already closed
 * @retval -EAGAIN Trace empty, try again
 * @retval -ENOSPC event buffer too small
 */
int octf_trace_pop(octf_trace_t trace, void *event, uint32_t *size);

/**
 * @brief Retrieves current free space of the circular buffer in bytes
 *
 * @param trace[in] Trace handle
 * @retval >=0 Number of free bytes
 * @retval -EINVAL Trace is invalid
 */
int64_t octf_trace_get_free_space(octf_trace_t trace);

/**
 * Checks if trace is almost full
 *
 * Sometimes it doesn't make any sense to ready a few events and suspend. It
 * may cost more CPU usage. It's better to wait longer until trace is almost
 * full.
 *
 * @param trace[in] Trace handle
 *
 * @retval 0 trace is not almost full
 * @retval 1 trace is almost full
 * @retval -EINVAL Trace handle is invalid
 */
int octf_trace_is_almost_full(octf_trace_t trace);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_OCTF_TRACE_TRACE_H
