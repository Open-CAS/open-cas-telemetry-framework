/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __KERNEL__
#include <octf/trace/internal/trace_env_usr.h>
#include <octf/trace/trace.h>
#else
#include "trace.h"
#include "trace_env_kernel.h"
#endif

#define TRACE_VER 1

#define TRACE_MAGIC(a, b, c, d)                                       \
    ((UINT64_C(a) << 24) | (UINT64_C(b) << 16) | (UINT64_C(c) << 8) | \
     UINT64_C(d))

/**
 * Magic number identifies trace buffer like in case of superblock in filesystem
 */
#define TRACE_MAGIC_BUFFER TRACE_MAGIC(0x1410, 0x1683, 0x1920, 0x1989)

struct octf_trace {
    octf_trace_open_mode_t mode;
    struct trace_producer_hdr *phdr;
    struct trace_consumer_hdr *chdr;
    void *ring_buffer;
    uint64_t ring_size;
    uint64_t ring_size_almost_empty;
};

struct trace_producer_hdr {
    env_atomic64 magic;
    env_atomic64 version;
    env_atomic64 closed;
    env_atomic64 wr_ptr;
    env_atomic64 wr_lock;
    env_atomic64 lost;
    env_atomic64 has_chdr;
};

struct trace_consumer_hdr {
    union {
        struct {
            env_atomic64 rd_ptr;
            env_atomic64 rd_lock;
        };
        octf_trace_hdr_t alignment;
    };
};

struct trace_event_hdr {
    /**
     * Bit indicates event ready and data has been copied
     */
    uint32_t ready : 1;

    /**
     * Size of data
     */
    uint32_t data_size;

    /**
     * Position of data in circular buffer
     */
    uint64_t data_ptr;
} __attribute__((packed, aligned(8)));

#define TRACE_ALIGNMENT sizeof(struct trace_event_hdr)

#define TRACE_ALIGN(s) (DIV_ROUND_UP((s), TRACE_ALIGNMENT) * TRACE_ALIGNMENT)

static bool _is_trace_valid(octf_trace_t trace) {
    if (!trace) {
        return false;
    }

    if (!trace->phdr) {
        return false;
    }

    if (env_atomic64_read(&trace->phdr->magic) != TRACE_MAGIC_BUFFER) {
        return false;
    }

    if (env_atomic64_read(&trace->phdr->version) != TRACE_VER) {
        return false;
    }

    return true;
}

static int _init_ring(octf_trace_t trace,
                      void *mempool,
                      size_t size,
                      octf_trace_hdr_t *hdr) {
    const uint64_t p_hdr_size = TRACE_ALIGN(sizeof(struct trace_producer_hdr));
    const uint64_t c_hdr_size = TRACE_ALIGN(sizeof(struct trace_consumer_hdr));

    if (size < (p_hdr_size + c_hdr_size)) {
        return -ENOSPC;
    }

    // Producer header starts at buffer beginning
    trace->phdr = mempool;
    size -= p_hdr_size;

    if (!hdr) {
        // consumer header in the main buffer
        trace->chdr = mempool + p_hdr_size;
        size -= c_hdr_size;
        trace->ring_buffer = mempool + p_hdr_size + c_hdr_size;
    } else {
        // consumer header in a separate buffer
        trace->chdr = (struct trace_consumer_hdr *) hdr;
        trace->ring_buffer = mempool + p_hdr_size;
    }

    // Set ring size considering trace alignment
    trace->ring_size = size;
    trace->ring_size /= TRACE_ALIGNMENT;
    trace->ring_size *= TRACE_ALIGNMENT;
    trace->ring_size_almost_empty = trace->ring_size * 2 / 10;

    return 0;
}

static int _init_producer(struct octf_trace *trace,
                          void *buffer,
                          size_t size,
                          octf_trace_hdr_t *hdr) {
    int result = _init_ring(trace, buffer, size, hdr);

    if (result) {
        return result;
    }

    //  Zero buffers
    memset_s(buffer, size, 0);
    if (hdr) {
        memset_s(hdr, sizeof(*hdr), 0);
    }

    trace->mode = octf_trace_open_mode_producer;
    env_atomic64_set(&trace->phdr->magic, TRACE_MAGIC_BUFFER);
    env_atomic64_set(&trace->phdr->version, TRACE_VER);
    env_atomic64_set(&trace->phdr->has_chdr, NULL != hdr);

    return 0;
}

static int _init_consumer(struct octf_trace *trace,
                          void *buffer,
                          size_t size,
                          octf_trace_hdr_t *hdr) {
    int result = _init_ring(trace, buffer, size, hdr);

    if (result) {
        return result;
    }

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    // Check if producer and customer are created with external headers or not
    if (!!env_atomic64_read(&trace->phdr->has_chdr) != !!hdr) {
        return -EINVAL;
    }

    trace->mode = octf_trace_open_mode_consumer;
    return 0;
}

int octf_trace_open(void *mempool,
                    uint32_t mempool_size,
                    octf_trace_hdr_t *hdr,
                    octf_trace_open_mode_t mode,
                    octf_trace_t *_trace) {
    int result;
    struct octf_trace *trace;

    if (mempool_size < OCTF_TRACE_MIN_BUFFER_SIZE) {
        return -ENOBUFS;
    }

    trace = env_zalloc(sizeof(*trace));
    if (!trace) {
        return -ENOMEM;
    }

    if (mode == octf_trace_open_mode_producer) {
        result = _init_producer(trace, mempool, mempool_size, hdr);
    } else if (mode == octf_trace_open_mode_consumer) {
        result = _init_consumer(trace, mempool, mempool_size, hdr);
    } else {
        result = -EINVAL;
    }

    if (result) {
        env_free(trace);
        return result;
    }

    *_trace = trace;
    return 0;
}

void octf_trace_close(octf_trace_t *trace) {
    struct octf_trace *_trace = *trace;

    if (!_trace) {
        // Trace already freed nothing to do
        return;
    }

    if (_trace->mode == octf_trace_open_mode_producer) {
        if (_is_trace_valid(_trace)) {
            env_atomic64_set(&_trace->phdr->closed, 1);
        } else {
            ENV_WARN(true, "Attempt of closing not initialized trace");
        }
    }

    env_free(_trace);
    *trace = NULL;
}

//******************************************************************************
// Internal auxiliary functions
//******************************************************************************

static inline bool _integrity_check(struct octf_trace *trace,
                                    uint64_t ptr,
                                    uint64_t count) {
    if (ptr >= trace->ring_size) {
        return false;
    }

    if (ptr + count > trace->ring_size) {
        return false;
    }

    if (ptr + count < count) {
        return false;
    }

    return true;
}

static uint64_t _get_continuous_space(struct octf_trace *trace,
                                      uint64_t rdp,
                                      uint64_t wrp) {
    uint64_t space = 0;

    if (wrp >= rdp) {
        space = trace->ring_size - wrp;
        if (!rdp) {
            space--;
        }
    } else {
        space = rdp - wrp - 1;
    }

    return space;
}

static uint64_t _get_free_space(struct octf_trace *trace,
                                uint64_t rdp,
                                uint64_t wrp) {
    uint64_t space;

    if (wrp >= rdp) {
        space = trace->ring_size - (wrp - rdp);
    } else {
        space = rdp - wrp;
    }

    return space - 1;
}

static uint64_t _move_ptr(struct octf_trace *trace, uint64_t ptr, uint64_t mv) {
    ptr += mv;
    if (ptr >= trace->ring_size) {
        ptr %= trace->ring_size;
    }

    return ptr;
}

//******************************************************************************
// PUSH
//******************************************************************************

static int _try_lock_wr(octf_trace_t trace) {
    long lock = -1;
    uint32_t retry = 0;

    do {
        if (retry++ > 128) {
            return -EBUSY;
        }

        // Try lock for reading event
        lock = env_atomic64_cmpxchg(&trace->phdr->wr_lock, 0, 1);
    } while (lock != 0);

    return 0;
}

static void _unlock_wr(octf_trace_t trace) {
    env_atomic64_set(&trace->phdr->wr_lock, 0);
}

static uint64_t _is_wrap(uint64_t rdp, uint64_t wrp) {
    return wrp > rdp;
}

static struct trace_event_hdr *_allocate_event(struct octf_trace *trace,
                                               const uint32_t size) {
    uint32_t _size;
    uint64_t wrp, rdp, continuous;
    struct trace_event_hdr *hdr = NULL, *result = NULL;

    if (_try_lock_wr(trace)) {
        return NULL;
    }

    // Check if ring buffer has enough space for writing this event (event
    // header + event data)

    wrp = env_atomic64_read(&trace->phdr->wr_ptr);
    rdp = env_atomic64_read(&trace->chdr->rd_ptr);

    // First allocate space for trace header
    if (sizeof(*hdr) > _get_continuous_space(trace, rdp, wrp)) {
        goto END;
    }

    // Check if header doesn't exceed ring buffer
    if (wrp + sizeof(*hdr) > trace->ring_size) {
        ENV_BUG();
        goto END;
    }

    hdr = (struct trace_event_hdr *) (trace->ring_buffer + wrp);
    wrp = _move_ptr(trace, wrp, sizeof(*hdr));

    // Allocate continuous space for trace data
    _size = TRACE_ALIGN(size);
    continuous = _get_continuous_space(trace, rdp, wrp);
    if (_size > continuous) {
        if (_is_wrap(rdp, wrp) && (rdp != 0)) {
            // We are at the end of circular buffer and not able to find enough
            // space in it. So we will move pointer at the beginning of ring
            // buffer and check if there is enough space for trace data.
            wrp = _move_ptr(trace, wrp, continuous);
            // Now write pointer pointer has to be zero
            ENV_BUG_ON(0 != wrp);

            if (_size > _get_continuous_space(trace, rdp, wrp)) {
                // Still not enough space for trace data
                goto END;
            }
        } else {
            // Not enough space for trace data, fail trace allocation
            goto END;
        }
    }

    // Check if data buffer
    if (wrp + _size > trace->ring_size) {
        ENV_BUG();
        goto END;
    }

    // Prepare header
    hdr->ready = false;
    hdr->data_ptr = wrp;
    hdr->data_size = size;
    result = hdr;

    // Update write pointer pointer
    wrp = _move_ptr(trace, wrp, _size);
    env_atomic64_set(&trace->phdr->wr_ptr, wrp);

END:
    _unlock_wr(trace);
    return result;
}

int octf_trace_push(octf_trace_t trace,
                    const void *event,
                    const uint32_t size) {
    struct trace_event_hdr *hdr;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_producer) {
        return -EINVAL;
    }

    // Allocate trace
    hdr = _allocate_event(trace, size);
    if (!hdr) {
        // Not enough space for storing trace event
        env_atomic64_inc(&trace->phdr->lost);
        return -ENOSPC;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        // Invalidate trace and stop pushing and popping
        env_atomic64_set(&trace->phdr->magic, 0);
        ENV_BUG();
        return -EINVAL;
    }

    // Copy data
    memcpy_s(trace->ring_buffer + hdr->data_ptr, hdr->data_size, event, size);

    // Commit event
    hdr->ready = true;

    return 0;
}

int octf_trace_get_wr_buffer(octf_trace_t trace,
                             octf_trace_event_handle_t *ev_hndl,
                             void **event,
                             const uint32_t size) {
    struct trace_event_hdr *hdr;
    *ev_hndl = NULL;
    *event = NULL;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_producer) {
        return -EINVAL;
    }

    // Allocate trace
    hdr = _allocate_event(trace, size);
    if (!hdr) {
        // Not enough space for storing trace event
        env_atomic64_inc(&trace->phdr->lost);
        return -ENOSPC;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        // Invalidate trace and stop pushing and popping
        env_atomic64_set(&trace->phdr->magic, 0);
        ENV_BUG();
        return -EINVAL;
    }

    *ev_hndl = hdr;
    *event = trace->ring_buffer + hdr->data_ptr;

    return 0;
}

int octf_trace_commit_wr_buffer(octf_trace_t trace,
                                octf_trace_event_handle_t ev_hndl) {
    struct trace_event_hdr *hdr = ev_hndl;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_producer) {
        return -EINVAL;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        // Invalidate trace and stop pushing and popping
        env_atomic64_set(&trace->phdr->magic, 0);
        ENV_BUG();
        return -EINVAL;
    }

    hdr->ready = true;

    return 0;
}

//******************************************************************************
// POP
//******************************************************************************

bool _is_empty(uint64_t rdp, uint64_t wrp) {
    return rdp == wrp;
}

static struct trace_event_hdr *_get_rd_hdr(struct octf_trace *trace) {
    uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
    uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
    struct trace_event_hdr *hdr;

    // Check if trace is empty
    if (_is_empty(ptr_rd, ptr_wr)) {
        // No event to be read
        return NULL;
    }

    if (!_integrity_check(trace, ptr_rd, sizeof(*hdr))) {
        // Inconsistent trace state, trying to access out of ring buffer
        ENV_BUG();
        return NULL;
    }

    hdr = (struct trace_event_hdr *) (trace->ring_buffer + ptr_rd);

    if (!hdr->ready) {
        // Event not ready
        return NULL;
    }

    return hdr;
}

static void _move_rd_ptr(struct octf_trace *trace,
                         struct trace_event_hdr *hdr) {
    // Move pointer to next event which will be read
    uint64_t rd_ptr =
            _move_ptr(trace, hdr->data_ptr, TRACE_ALIGN(hdr->data_size));

    env_atomic64_set(&trace->chdr->rd_ptr, rd_ptr);
}

static int _try_lock_rd(octf_trace_t trace) {
    // Try lock for reading event
    int lock = env_atomic64_cmpxchg(&trace->chdr->rd_lock, 0, 1);

    if (lock != 0) {
        // Other thread took lock, return busy
        return -EBUSY;
    }

    return 0;
}

static void _unlock_rd(octf_trace_t trace) {
    env_atomic64_set(&trace->chdr->rd_lock, 0);
}

int octf_trace_pop(octf_trace_t trace, void *event, uint32_t *size) {
    int result = -1;
    struct trace_event_hdr *hdr;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_consumer) {
        return -EPERM;
    }

    // Try lock for reading event
    if (_try_lock_rd(trace)) {
        // Other thread took lock, return
        return -EBUSY;
    }

    // Get read position
    hdr = _get_rd_hdr(trace);
    if (!hdr) {
        // No event to be read
        if (env_atomic64_read(&trace->phdr->closed)) {
            result = -EBADF;
        } else {
            result = -EAGAIN;
        }
        goto END;
    }

    if (*size < hdr->data_size) {
        // No enough space in user buffer to copy event
        result = -ENOSPC;
        goto END;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        result = -EINVAL;
        ENV_BUG();
        goto END;
    }

    // Copy event
    memcpy_s(event, *size, trace->ring_buffer + hdr->data_ptr, hdr->data_size);

    // Update size
    *size = hdr->data_size;

    // Remove event from trace
    _move_rd_ptr(trace, hdr);

    result = 0;
END:
    _unlock_rd(trace);
    return result;
}

int octf_trace_get_rd_buffer(octf_trace_t trace,
                             octf_trace_event_handle_t *ev_hndl,
                             void **event,
                             uint32_t *size) {
    int result = -1;
    struct trace_event_hdr *hdr;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_consumer) {
        return -EPERM;
    }

    // Try lock for reading event
    if (_try_lock_rd(trace)) {
        // Other thread took lock, return
        return -EBUSY;
    }

    // Get read position
    hdr = _get_rd_hdr(trace);
    if (!hdr) {
        // No event to be read
        if (env_atomic64_read(&trace->phdr->closed)) {
            result = -EBADF;
        } else {
            result = -EAGAIN;
        }
        goto END;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        result = -EINVAL;
        ENV_BUG();
        goto END;
    }

    // Update size
    *size = hdr->data_size;
    *ev_hndl = hdr;
    *event = trace->ring_buffer + hdr->data_ptr;
    return 0;

END:
    _unlock_rd(trace);
    return result;
}

int octf_trace_release_rd_buffer(octf_trace_t trace,
                                 octf_trace_event_handle_t ev_hndl) {
    struct trace_event_hdr *hdr = ev_hndl;

    if (!_is_trace_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_consumer) {
        return -EINVAL;
    }

    if (!_integrity_check(trace, hdr->data_ptr, hdr->data_size)) {
        // Inconsistent trace state, trying to access out of ring buffer
        // Invalidate trace and stop pushing and popping
        env_atomic64_set(&trace->phdr->magic, 0);
        ENV_BUG();
        return -EINVAL;
    }

    _move_rd_ptr(trace, hdr);
    _unlock_rd(trace);
    return 0;
}

int octf_trace_is_empty(const octf_trace_t trace) {
    if (_is_trace_valid(trace)) {
        uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
        uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);

        if (_is_empty(ptr_rd, ptr_wr)) {
            return 1;
        } else {
            return 0;
        }

    } else {
        return -EINVAL;
    }
}

int octf_trace_is_almost_full(const octf_trace_t trace) {
    if (_is_trace_valid(trace)) {
        uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
        uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
        uint64_t free_space = _get_free_space(trace, ptr_rd, ptr_wr);

        if (free_space < trace->ring_size_almost_empty) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -EINVAL;
    }
}

int octf_trace_is_closed(const octf_trace_t trace) {
    if (_is_trace_valid(trace)) {
        if (env_atomic64_read(&trace->phdr->closed)) {
            return 1;
        } else {
            return 0;
        }

    } else {
        return -EINVAL;
    }
}

int64_t octf_trace_get_lost_count(const octf_trace_t trace) {
    if (_is_trace_valid(trace)) {
        return env_atomic64_read(&trace->phdr->lost);
    } else {
        return -EINVAL;
    }
}

int64_t octf_trace_get_free_space(const octf_trace_t trace) {
    if (_is_trace_valid(trace)) {
        uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
        uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
        return _get_free_space(trace, ptr_rd, ptr_wr);
    } else {
        return -EINVAL;
    }
}
