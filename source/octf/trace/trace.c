/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __KERNEL__
#include <octf/trace/trace.h>
#include <octf/trace/internal/trace_env_usr.h>
#else
#include "trace.h"
#include "internal/trace_env_kernel.h"
#endif


#define TRACE_VER               1

#define TRACE_MAGIC(a,b,c,d)    ((UINT64_C(a) << 24) | (UINT64_C(b) << 16) \
                                    | (UINT64_C(c) << 8) | UINT64_C(d))

/**
 * Magic number identifies trace buffer like in case of superblock in filesystem
 */
#define TRACE_MAGIC_BUFFER      TRACE_MAGIC(0x1410, 0x1683, 0x1920, 0x1989)

#define TRACE_ALIGNMENT         8ULL

#define TRACE_ALIGN(s)          (DIV_ROUND_UP(s, TRACE_ALIGNMENT) \
                                    * TRACE_ALIGNMENT)

struct octf_trace {
    octf_trace_open_mode_t mode;
    struct trace_producer_hdr *phdr;
    struct trace_consumer_hdr *chdr;
    void *ring_buffer;
    uint64_t ring_size;
};

struct trace_producer_hdr {
    env_atomic64 magic;
    env_atomic64 version;
    env_atomic64 closed;
    env_atomic64 wr_ptr;
    env_atomic64 wr_lock;
    env_atomic64 rd_ptr;
    env_atomic64 rd_lock;
    env_atomic64 lost;
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
    uint32_t size;
    uint32_t ready;
};

static bool _trace_is_valid(octf_trace_t trace)
{
    if (!trace) {
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

static int _trace_init_handle(octf_trace_t trace, void *mempool, size_t size,
        octf_trace_hdr_t *hdr)
{
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
        trace->chdr = (struct trace_consumer_hdr *)hdr;
        trace->ring_buffer = mempool + p_hdr_size;
    }

    trace->ring_size = size;

    return 0;
}

static int _trace_init_producer(struct octf_trace *trace, void *buffer,
        size_t size, octf_trace_hdr_t *hdr)
{
    int result = _trace_init_handle(trace, buffer, size, hdr);

    if (result) {
        return result;
    }

    //  Zero buffer
    memset(buffer, 0, size);

    trace->mode = octf_trace_open_mode_producer;
    env_atomic64_set(&trace->phdr->magic, TRACE_MAGIC_BUFFER);
    env_atomic64_set(&trace->phdr->version, TRACE_VER);

    // Move read pointer before write
    env_atomic64_set(&trace->chdr->rd_ptr, trace->ring_size - 1);

    return 0;
}

static int _trace_init_consumer(struct octf_trace *trace, void *buffer,
        size_t size, octf_trace_hdr_t *hdr)
{
    int result = _trace_init_handle(trace, buffer, size, hdr);

    if (result) {
        return result;
    }

    if (!_trace_is_valid(trace)) {
        return -EINVAL;
    }

    trace->mode = octf_trace_open_mode_consumer;
    return 0;
}

int octf_trace_open(void *mempool, uint32_t mempool_size, octf_trace_hdr_t *hdr,
        octf_trace_open_mode_t mode, octf_trace_t *_trace)
{
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
        result = _trace_init_producer(trace, mempool, mempool_size, hdr);
    } else if (mode == octf_trace_open_mode_consumer) {
        result = _trace_init_consumer(trace, mempool, mempool_size, hdr);
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

void octf_trace_close(octf_trace_t *trace)
{
    struct octf_trace *_trace = *trace;

    if (!_trace) {
        // Trace already freed nothing to do
        return;
    }

    if (_trace->mode == octf_trace_open_mode_producer) {
        if (_trace_is_valid(_trace)) {
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

/**
 * @brief Integrity check for write/read/move operation
 *
 * It ensures that pointer for write/read/move is within trace buffer
 *
 * @param trace Trace handle
 * @param ptr pointer within trace buffer
 */
static inline void _trace_integrity_check(struct octf_trace *trace,
        const void *ptr)
{
    // Must be in range of trace buffer, check it
    if (ptr < trace->ring_buffer || ptr >= trace->ring_buffer + trace->ring_size) {
        ENV_BUG();
    }
}

typedef void (*trace_op_pfn)(void *trace_pos, void *buf, uint32_t size);

/**
 * @brief Helper function to iterate through trace buffer for given
 *	  number of bytes and perform provided operation
 * @param trace Trace instance
 * @param trace_pos Pointer at location whithin trace buffer
 * @param buf Pointer to data buffer
 * @param size Number of bytes to process
 * @param op operation handler function
 *
 * @return location within trace buffer after performing operation
 */
static void* _trace_op(struct octf_trace *trace, void *trace_pos,
    void *buf, uint32_t size, trace_op_pfn op)
{
    _trace_integrity_check(trace, trace_pos);

    while (size) {
        uint32_t count = MIN((uint64_t)size,
            (uint64_t)(trace->ring_buffer + trace->ring_size - trace_pos));

        if (op) {
            op(trace_pos, buf, count);
        }

        trace_pos += count;
        buf += count;

        if (trace_pos >= trace->ring_buffer + trace->ring_size)
            trace_pos = trace->ring_buffer;

        size -= count;
    }

    return trace_pos;
}

static void _wr_op(void *trace_dst, void *buf, uint32_t size) {
    memcpy(trace_dst, buf, size);
}

/**
 * @brief Writes a portion of data into trace buffer at specified location
 * within trace's buffer
 *
 * @param trace Trace instance
 * @param dst Pointer at location where starts writing into trace's buffer
 * @param src Source data to be written into buffer
 * @param size Number of bytes to be written
 */
static void _trace_wr(struct octf_trace *trace, void *dst, const void *src,
        uint32_t size)
{
	_trace_op(trace, dst, (void *)src, size, _wr_op);
}

static void _rd_op(void *trace_dst, void *buf, uint32_t size)
{
    memcpy(buf, trace_dst, size);
}

/**
 * @brief Reads a portion of data from trace buffer at specified location
 * within trace's buffer
 *
 * @param trace Trace instance
 * @param src Pointer at location within trace's buffer from where starts
 * reading data
 * @param dst Output memory buffer where store read data
 * @param size Number of bytes to be read
 */
static void _trace_rd(struct octf_trace *trace, const void *src, void *dst,
        uint32_t size)
{
     _trace_op(trace, (void *)src, dst, size, _rd_op);
}

/**
 * @brief Returns new pointer after moving specified pointer within trace's
 * buffer by number of bytes
 *
 * @param trace Trace instance
 * @param base Starting position for moving pointer
 * @param size Number of bytes to be moved into new position
 *
 * @retval new Position within trace's buffer moved by number of bytes
 */
static void* _trace_move(struct octf_trace *trace, void *base, uint32_t size)
{
    return _trace_op(trace, base, NULL, size, NULL);
}

static uint64_t _trace_get_free_space(struct octf_trace *trace, uint64_t rdp,
        uint64_t wrp)
{
    uint64_t space;

    if (wrp > rdp) {
        space = trace->ring_size - (wrp - rdp);
    } else if (rdp >= wrp){
        space = rdp - wrp;
    }

    return space - 1;
}

static uint64_t _trace_event_size(struct trace_event_hdr *hdr)
{
    return TRACE_ALIGN(sizeof(struct trace_event_hdr) + hdr->size);
}

static void _zero_op(void *trace_dst, void *ignore, uint32_t size)
{
    (void)ignore;
    memset(trace_dst, 0, size);
}

/**
 * @brief zeros region within trace's buffer
 *
 * @param trace Trace instance
 * @param base Position within trace's buffer at which starts zeroing
 * @param size Number of bytes to be zeroed
 */
static void _trace_zero(struct octf_trace *trace, void *base, uint32_t size)
{
    _trace_op(trace, base, NULL, size, _zero_op);
}

//******************************************************************************
// PUSH
//******************************************************************************

static int octf_trace_try_lock_wr(octf_trace_t trace) {
    long lock = -1;
    uint32_t try = 0;

    do {
        // Try lock for reading event
        lock = env_atomic64_cmpxchg(&trace->phdr->wr_lock, 0, 1);

        if (try++ > 128) {
            return -EBUSY;
        }

    } while (lock != 0);

    return 0;
}

static void octf_trace_unlock_wr(octf_trace_t trace) {
    env_atomic64_set(&trace->phdr->wr_lock, 0);
}

static void *_trace_get_wr_pos(struct octf_trace *trace, uint32_t size)
{
    size_t _size;
    uint64_t ptr_wr, ptr_rd;
    void *pos = NULL;

    // Calculate size including event header and alignment
    _size = TRACE_ALIGN(sizeof(struct trace_event_hdr) + size);

    if (octf_trace_try_lock_wr(trace)) {
        return NULL;
    }

    // Check if buffer has enough space for writing this event
    ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
    ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);

    if (_size > _trace_get_free_space(trace, ptr_rd, ptr_wr)) {
        octf_trace_unlock_wr(trace);
        return NULL;
    }

    //  Get write position
    pos = trace->ring_buffer + ptr_wr;

    //  Clear event header
    _trace_zero(trace, pos, sizeof(struct trace_event_hdr));

    //  Calculate new write position
    ptr_wr += _size;
    if (ptr_wr >= trace->ring_size) {
        ptr_wr %= trace->ring_size;
    }

    //  Update write pointer
    env_atomic64_set(&trace->phdr->wr_ptr, ptr_wr);

    octf_trace_unlock_wr(trace);

    return pos;
}

int octf_trace_push(octf_trace_t trace, const void *event, const uint32_t size)
{
    struct trace_event_hdr hdr;
    void *pos;

    if (!_trace_is_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_producer) {
        return -EINVAL;
    }

    // Find position where store new event
    pos = _trace_get_wr_pos(trace, size);
    if (!pos) {
        env_atomic64_inc(&trace->phdr->lost);
        return -ENOSPC;
    }

    // Copy event content
    _trace_wr(trace, _trace_move(trace, pos, sizeof(hdr)), event, size);

    // Set header
    hdr.size = size;
    hdr.ready = 1; //  TODO (mbarczak) check if 'ready' field is copied as last

    // Copy header
    _trace_wr(trace, pos, &hdr, sizeof(hdr));

    return 0;
}

//******************************************************************************
// POP
//******************************************************************************

bool _trace_is_empty(struct octf_trace *trace, uint64_t rdp, uint64_t wrp)
{
    rdp++;
    if (rdp == trace->ring_size) {
        rdp = 0;
    }

    return rdp == wrp;
}

static void *_rd_pos(struct octf_trace *trace)
{
    uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr) + 1;
    void *pos;

    if (ptr_rd == trace->ring_size) {
        ptr_rd = 0;
    }

    pos = trace->ring_buffer + ptr_rd;

    return pos;
}

static void *trace_get_rd_pos(struct octf_trace *trace, struct trace_event_hdr *hdr)
{
    uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
    uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
    void *pos = _rd_pos(trace);

    // Check if trace is empty
    if (_trace_is_empty(trace, ptr_rd, ptr_wr)) {
        // No event to be read
        return NULL;
    }

    // Read header of event
    _trace_rd(trace, pos, hdr, sizeof(*hdr));

    if (!hdr->ready) {
        // Event not ready
        return NULL;
    }

    return pos;
}

static void _trace_move_rd_ptr(struct octf_trace *trace,
        struct trace_event_hdr *hdr)
{
    // Get size of of current event
    uint64_t size = _trace_event_size(hdr);

    // Get current read position
    uint64_t rd_ptr = env_atomic64_read(&trace->chdr->rd_ptr);

    // Calculate new read position
    rd_ptr += size;
    if (rd_ptr >= trace->ring_size) {
        rd_ptr %= trace->ring_size;
    }

    // Move pointer to next event which will be read
    env_atomic64_set(&trace->chdr->rd_ptr, rd_ptr);
}

static int octf_trace_try_lock_rd(octf_trace_t trace) {
    // Try lock for reading event
    int lock = env_atomic64_cmpxchg(&trace->chdr->rd_lock, 0, 1);

    if (lock != 0) {
        // Other thread took lock, return
        return -EBUSY;
    }

    return 0;
}

static void octf_trace_unlock_rd(octf_trace_t trace) {
    env_atomic64_set(&trace->chdr->rd_lock, 0);
}

int octf_trace_pop(octf_trace_t trace, void *event, uint32_t *size)
{
    struct trace_event_hdr hdr;
    void *pos;

    if (!_trace_is_valid(trace)) {
        return -EINVAL;
    }

    if (trace->mode != octf_trace_open_mode_consumer) {
        return -EPERM;
    }

    // Try lock for reading event
    if (octf_trace_try_lock_rd(trace)) {
        // Other thread took lock, return
        return -EBUSY;
    }

    // Get read position
    pos = trace_get_rd_pos(trace, &hdr);
    if (!pos) {
        int result;

        if (env_atomic64_read(&trace->phdr->closed)) {
            result = -EBADF;
        } else {
            result = -EAGAIN;
        }

        // No event to be read
        env_atomic64_set(&trace->chdr->rd_lock, 0);
        return result;
    }

    if (*size < hdr.size) {
        // No enough space in user buffer to copy event
        env_atomic64_set(&trace->chdr->rd_lock, 0);
        return -ENOSPC;
    }

    // Copy event
    _trace_rd(trace, _trace_move(trace, pos, sizeof(hdr)), event, hdr.size);

    // Remove event from trace
    _trace_move_rd_ptr(trace, &hdr);

    // Unlock RD lock
    octf_trace_unlock_rd(trace);

    *size = hdr.size;

    return 0;
}

int octf_trace_is_empty(octf_trace_t trace)
{
    if (_trace_is_valid(trace)) {

        uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
        uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);

        if (_trace_is_empty(trace, ptr_rd, ptr_wr)) {
            return 1;
        } else {
            return 0;
        }

    } else {
        return -EINVAL;
    }
}

int octf_trace_is_closed(octf_trace_t trace)
{
    if (_trace_is_valid(trace)) {

        if (env_atomic64_read(&trace->phdr->closed)) {
            return 1;
        } else {
            return 0;
        }

    } else {
        return -EINVAL;
    }
}

int64_t octf_trace_get_lost_count(octf_trace_t trace)
{
    if (_trace_is_valid(trace)) {
        return env_atomic64_read(&trace->phdr->lost);
    } else {
        return -EINVAL;
    }
}

int64_t octf_trace_get_free_space(octf_trace_t trace)
{
    if (_trace_is_valid(trace)) {
        uint64_t ptr_wr = env_atomic64_read(&trace->phdr->wr_ptr);
        uint64_t ptr_rd = env_atomic64_read(&trace->chdr->rd_ptr);
        return _trace_get_free_space(trace, ptr_rd, ptr_wr);
    }else {
        return -EINVAL;
    }
}
