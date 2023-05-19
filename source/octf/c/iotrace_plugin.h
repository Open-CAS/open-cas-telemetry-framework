/*
 * Copyright(c) 2012-2018 Intel Corporation
 * Copyright 2023 Solidigm All Rights Reserved
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_C_IOTRACE_PLUGIN_H
#define SOURCE_OCTF_C_IOTRACE_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <octf/trace/iotrace_event.h>

/**
 * @brief IO trace plug-in handle
 */
struct octf_iotrace_plugin_context {
    /**
     * Flag indicating tracing is active
     */
    volatile bool tracing_active;

    /**
     * Reference of trace event SID for sequentializing them
     */
    volatile uint64_t ref_sid;

    /**
     * IO tracer plugin
     */
    void *plugin;
};

/**
 * @brief IO trace plug-in handle
 */
typedef struct octf_iotrace_plugin_context *octf_iotrace_plugin_context_t;

/**
 * Checks if tracing is active
 *
 * @param plugin IO trace plugin handle
 *
 * @retval true tracing is active
 * @retval false tracing is not active
 */
static inline bool octf_iotrace_plugin_is_tracing_active(
        octf_iotrace_plugin_context_t plugin) {
    return plugin->tracing_active;
}

/**
 * IO trace plug-in configuration
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
 * @param[out] plugin_context context of IO trace plug-in
 *
 * @return operation status
 * @retval 0 - operation successful
 * @retval Non-zero - operation failure while creating IO tracer plug-in
 */
int octf_iotrace_plugin_create(const struct octf_iotrace_plugin_cnfg *cnfg,
                               octf_iotrace_plugin_context_t *plugin_context);

/**
 * @brief Destroys IO tracer plug-in
 *
 * @param plugin IO tracer plug-in context to be destroyed
 */
void octf_iotrace_plugin_destroy(octf_iotrace_plugin_context_t *plugin_context);

/**
 * @brief Initializes IO trace event header
 *
 * @param plugin IO tracer plug-in context
 * @param hdr IO trace header to be initialized
 * @param type IO trace event type
 * @param size entire Size of IO trace event including header
 */
void octf_iotrace_plugin_init_trace_header(
        octf_iotrace_plugin_context_t plugin_context,
        struct iotrace_event_hdr *hdr,
        iotrace_event_type type,
        uint32_t size);
/**
 * @brief Pushes IO trace event
 *
 * @note Pushed events shall be started with IO trace event header
 * (struct iotrace_event_hdr).
 * @note Events defined in iotrace_event.h are serialized only.
 *
 * @param plugin IO tracer plug-in context
 * @param ioQueue IO queue id into which store event
 * @param trace Trace event to be stored
 * @param size Size of trace event to be stored
 */
void octf_iotrace_plugin_push_trace(
        octf_iotrace_plugin_context_t plugin_context,
        uint32_t ioQueue,
        const void *trace,
        size_t size);

/**
 * @brief Main function wrapper for C CLI programs
 *
 * @param app_name Application name
 * @param app_version Application version
 * @param id Unique IO trace plug-in ID
 *
 * @return Main function return result
 */
int octf_iotrace_plugin_main(
        const char *app_name,
        const char *app_version,
        const char* id,
        int argc,
        char *argv[]);

#ifdef __cplusplus
}
#endif

#endif  // SOURCE_OCTF_C_IOTRACE_PLUGIN_H
