/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_C_IOTRACE_PLUGIN_H
#define SOURCE_OCTF_C_IOTRACE_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <octf/trace/iotrace_event.h>

typedef void *octf_iotrace_plugin_t;

struct octf_iotrace_plugin_cnfg {
    const char *id;
    uint32_t io_queue_count;
};

int octf_iotrace_plugin_create(const struct octf_iotrace_plugin_cnfg *cnfg,
                               octf_iotrace_plugin_t *plugin);

void octf_iotrace_plugin_destroy(octf_iotrace_plugin_t *plugin);

void octf_iotrace_plugin_init_trace_header(octf_iotrace_plugin_t plugin,
                                           struct iotrace_event_hdr *hdr,
                                           iotrace_event_type type,
                                           uint32_t size);

void octf_iotrace_plugin_push_trace(octf_iotrace_plugin_t plugin,
                                    uint32_t queue,
                                    const void *trace,
                                    size_t size);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_OCTF_C_IOTRACE_PLUGIN_H
