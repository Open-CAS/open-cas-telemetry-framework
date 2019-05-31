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

/**
 * @file
 *
 * C wrapper of IO tracer plug-in
 */

/**
 * @brief IO trace plug-in handle
 */
typedef void *octf_iotrace_plugin_t;

/**
 * IO trace configuration plug-in
 */
struct octf_iotrace_plugin_cnfg {
    /**
     * Unique IO trace plug-in ID
     */
    const char *id;

    /**
     * Number of IO queues to be traced
     */
    uint32_t io_queue_count;
};

/**
 * @brief Creates IO tracer plug-in
 *
 * @param cnfg IO tracer plug-in configuration
 * @param[out] plugin handle to IO trace plug-in
 *
 * @return operation status
 * @retval 0 - operation successful
 * @retval Non zero - operation failure while creating IO tracer plug-in
 */
int octf_iotrace_plugin_create(const struct octf_iotrace_plugin_cnfg *cnfg,
                               octf_iotrace_plugin_t *plugin);

/**
 * @brief Destroys IO tracer plug-in
 *
 * @param plugin IO tracer plug-in handle to be destroyed
 */
void octf_iotrace_plugin_destroy(octf_iotrace_plugin_t *plugin);

/**
 * @brief Initializes IO trace event header
 *
 * @param plugin IO tracer plug-in handle
 * @param hdr IO trace header to be initialized
 * @param type IO trace event type
 * @param size entire Size of IO trace event including header
 */
void octf_iotrace_plugin_init_trace_header(octf_iotrace_plugin_t plugin,
                                           struct iotrace_event_hdr *hdr,
                                           iotrace_event_type type,
                                           uint32_t size);
/**
 * @brief Pushes IO trace event
 *
 * @note Pushed evetns shall be started with IO trace event header
 * (struct iotrace_event_hdr).
 * @note Events defined in iotrace_event.h are serialized onlyt.
 *
 * @param plugin IO tracer plug-in handle
 * @param ioQueue IO queue id into which store event
 * @param trace Trace event to be stored
 * @param size Size of trace event to be stored
 *
 * @example Pushing an IO event
 * @code
 *  // Define IO trace event
 *  struct iotrace_event ev = {};
 *
 *  // Initialize trace header
 *  octf_iotrace_plugin_init_trace_header(plugin, &ev.hdr,
 * iotrace_event_type_io, sizeof(ev));
 *
 *  // Fill IO trace
 *  ev.lba = ...
 *  ev.len = ...
 *  ev.operation = ...
 *  ...
 *
 *  // Push  IO trace
 *  octf_iotrace_plugin_push_trace(plugin, queue_id, &ev, sizeof(ev));
 * @endcode
 */
void octf_iotrace_plugin_push_trace(octf_iotrace_plugin_t plugin,
                                    uint32_t ioQueue,
                                    const void *trace,
                                    size_t size);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_OCTF_C_IOTRACE_PLUGIN_H
