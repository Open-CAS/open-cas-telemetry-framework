/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <octf/c/iotrace_plugin.h>

static void trace_io(octf_iotrace_plugin_context_t context) {
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

volatile bool thread_stop;

static void *thread_function(void *data) {
    octf_iotrace_plugin_context_t context = data;

    while (!thread_stop) {
        if (octf_iotrace_plugin_is_tracing_active(context)) {
            trace_io(context);
        }

        usleep(1000 * (random() % 100));
    }

    return NULL;
}

void signal_handler(int signal_no) {
    printf("Interrupt signal received\n");
    thread_stop = true;
}

int main() {
    int result = 0;
    pthread_t thread;
    octf_iotrace_plugin_context_t context;

    struct octf_iotrace_plugin_cnfg cnfg = {
            .id = "c-iotrace-example",
            .io_queue_count = 1,
    };

    // Register signals
    if (signal(SIGINT, signal_handler) == SIG_ERR ||
        signal(SIGTERM, signal_handler) == SIG_ERR) {
        // Cannot register handler for signals
        return errno;
    }

    // Create plug-in
    result = octf_iotrace_plugin_create(&cnfg, &context);
    if (result) {
        return result;
    }

    // Run thread which generates IO traces
    result = pthread_create(&thread, NULL, thread_function, context);
    if (result) {
        octf_iotrace_plugin_destroy(&context);
        return result;
    }

    // Wait for thread end
    pthread_join(thread, NULL);

    // Destroy plugin
    octf_iotrace_plugin_destroy(&context);

    return result;
}
