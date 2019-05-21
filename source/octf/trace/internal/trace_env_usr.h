/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_INTERNAL_TRACE_ENV_USR_H
#define SOURCE_OCTF_TRACE_INTERNAL_TRACE_ENV_USR_H

// TODO (mbarczak) synchronize environment definition with latest CAS

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(x, y)                \
    ({                           \
        __typeof__(x) __x = (x); \
        __typeof__(y) __y = (y); \
        __x < __y ? __x : __y;   \
    })
#endif

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(x, y)       \
    ({                           \
        __typeof__(x) __x = (x); \
        __typeof__(y) __y = (y); \
        (__x + __y - 1) / __y;   \
    })
#endif

#define likely(cond) __builtin_expect(!!(cond), 1)
#define unlikely(cond) __builtin_expect(!!(cond), 0)

typedef struct {
    volatile long counter;
} env_atomic64;

static inline long env_atomic64_read(const env_atomic64 *a) {
    return a->counter;
}

static inline void env_atomic64_set(env_atomic64 *a, long i) {
    a->counter = i;
}

static inline void env_atomic64_add(long i, env_atomic64 *a) {
    __sync_add_and_fetch(&a->counter, i);
}

static inline long env_atomic64_add_return(long i, env_atomic64 *a) {
    return __sync_add_and_fetch(&a->counter, i);
}

static inline void env_atomic64_sub(long i, env_atomic64 *a) {
    __sync_sub_and_fetch(&a->counter, i);
}

static inline void env_atomic64_inc(env_atomic64 *a) {
    env_atomic64_add(1, a);
}

static inline void env_atomic64_dec(env_atomic64 *a) {
    env_atomic64_sub(1, a);
}

static inline long env_atomic64_cmpxchg(env_atomic64 *a, long old, long newv) {
    return __sync_val_compare_and_swap(&a->counter, old, newv);
}

static inline long env_atomic64_inc_return(env_atomic64 *a) {
    return env_atomic64_add_return(1, a);
}

static inline int env_atomic64_add_unless(env_atomic64 *a, int i, int u) {
    int c, old;
    c = env_atomic64_read(a);
    for (;;) {
        if (unlikely(c == (u)))
            break;
        old = env_atomic64_cmpxchg((a), c, c + (i));
        if (likely(old == c))
            break;
        c = old;
    }
    return c != (u);
}

static inline void *env_zalloc(size_t size) {
    void *ptr = malloc(size);

    if (ptr)
        memset(ptr, 0, size);

    return ptr;
}

static inline void env_free(const void *ptr) {
    free((void *) ptr);
}

#define ENV_WARN(cond, fmt...) fprintf(stderr, fmt)
#define ENV_WARN_ON(cond) ;
#define ENV_WARN_ONCE(cond, fmt...) ENV_WARN(cond, fmt)

#define ENV_BUG() assert(0)
#define ENV_BUG_ON(cond) assert(!(cond))

#endif  // SOURCE_OCTF_TRACE_INTERNAL_TRACE_ENV_USR_H
