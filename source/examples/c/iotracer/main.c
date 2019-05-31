/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/c/iotrace_plugin.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

static const uint32_t IO_QUEUE_COUNT = 4;

struct thread_ctrl {
    uint32_t queue_id;
    octf_iotrace_plugin_t iotrace_plugin;
    bool run;
};

static void _trace_io(octf_iotrace_plugin_t plugin, uint32_t queue_id)
{
    // Define IO trace event
    struct iotrace_event ev = {};

    // Initialize trace header
    octf_iotrace_plugin_init_trace_header(plugin, &ev.hdr, iotrace_event_type_io,
            sizeof(ev));

    // Fill IO trace
    ev.lba = random();
    ev.len = random() % 256;
    if (random() % 2) {
        ev.operation = iotrace_event_operation_wr;
    } else {
        ev.operation = iotrace_event_operation_rd;
    }

    // Push  IO trace
    octf_iotrace_plugin_push_trace(plugin, queue_id, &ev, sizeof(ev));
}

static void *thread(void *context)
{
    struct thread_ctrl *ctrl = (struct thread_ctrl *)(context);

    while (ctrl->run) {
        _trace_io(ctrl->iotrace_plugin, ctrl->queue_id);
        usleep(1000 * 1000);
    }

    return NULL;
}

int main() {
    uint32_t i;
    octf_iotrace_plugin_t plugin;
    struct octf_iotrace_plugin_cnfg cnfg = {
            .id = "c-iotrace-example",
            .io_queue_count = IO_QUEUE_COUNT
    };
    struct thread_ctrl ctrls[IO_QUEUE_COUNT];

    if (octf_iotrace_plugin_create(&cnfg, &plugin)) {
        return -1;
    }

    for (i = 0; i < IO_QUEUE_COUNT; i++) {
        ctrls[i].queue_id = i;
        ctrls[i].iotrace_plugin = plugin;
        ctrls[i].run = true;
    }

    thread(ctrls);

    octf_iotrace_plugin_destroy(&plugin);
}
