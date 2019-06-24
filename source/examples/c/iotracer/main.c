/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/c/iotrace_plugin.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void trace_io(octf_iotrace_plugin_context_t context)
{
    // Define IO trace event
    struct iotrace_event ev = {};

    // Initialize trace header
    octf_iotrace_plugin_init_trace_header(context, &ev.hdr,
            iotrace_event_type_io, sizeof(ev));

    // Fill IO trace
    ev.lba = random();
    ev.len = random() % 256;
    if (random() % 2) {
        ev.operation = iotrace_event_operation_wr;
    } else {
        ev.operation = iotrace_event_operation_rd;
    }

    // Push IO trace into queue 0
    octf_iotrace_plugin_push_trace(context, 0, &ev, sizeof(ev));
}

int main() {
    int result = 0;
    uint32_t i;
    octf_iotrace_plugin_context_t context;

    struct octf_iotrace_plugin_cnfg cnfg = {
            .id = "c-iotrace-example",
            .io_queue_count = 1,
    };

    if (octf_iotrace_plugin_create(&cnfg, &context)) {
        return -1;
    }

    while (true) {
        if (octf_iotrace_plugin_is_tracing_active(context)) {
            trace_io(context);
        }

        usleep(1000 * (random() % 100));
    }

    octf_iotrace_plugin_destroy(&context);

    return result;
}
