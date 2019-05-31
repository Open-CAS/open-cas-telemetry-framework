/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/c/iotrace_plugin.h>

#include <octf/trace/IOTracePlugin.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>

using namespace octf;

extern "C" int octf_iotrace_plugin_create(
        const struct octf_iotrace_plugin_cnfg *cnfg,
        octf_iotrace_plugin_t *_plugin) {
    IOTracePlugin *plugin = NULL;

    *_plugin = NULL;

    try {
        const char *prefix = "[OCTF]";
        log::cout << log::prefix << prefix;
        log::debug << log::prefix << prefix;
        log::verbose << log::prefix << prefix;
        log::cerr << log::prefix << prefix;
        log::critical << log::prefix << prefix;

        plugin = new IOTracePlugin(cnfg->id, cnfg->io_queue_count);

        if (!plugin->init()) {
            delete plugin;
            return -EINVAL;
        }

    } catch (Exception &e) {
        log::cerr << e.what() << std::endl;
        if (plugin) {
            delete plugin;
            return -EINVAL;
        }
    }

    log::cout << "Plugin " << plugin->getNodeId().getId() << " started"
              << std::endl;

    *_plugin = static_cast<void *>(plugin);
    return 0;
}

extern "C" void octf_iotrace_plugin_destroy(octf_iotrace_plugin_t *_plugin) {
    IOTracePlugin *plugin = NULL;

    if (_plugin) {
        plugin = static_cast<IOTracePlugin *>(*_plugin);
    }

    if (plugin) {
        try {
            plugin->deinit();
        } catch (Exception &e) {
            log::cerr << e.what() << std::endl;
        }

        delete plugin;
    }

    *_plugin = NULL;
}

extern "C" void octf_iotrace_plugin_init_trace_header(
        octf_iotrace_plugin_t _plugin,
        struct iotrace_event_hdr *hdr,
        iotrace_event_type type,
        uint32_t size) {
    IOTracePlugin *plugin = static_cast<IOTracePlugin *>(_plugin);
    if (plugin) {
        plugin->initTraceHeader(hdr, type, size);
    }
}

extern "C" void octf_iotrace_plugin_push_trace(octf_iotrace_plugin_t _plugin,
                                               uint32_t ioQueue,
                                               const void *trace,
                                               size_t size) {
    IOTracePlugin *plugin = static_cast<IOTracePlugin *>(_plugin);
    if (plugin) {
        plugin->push(ioQueue, trace, size);
    }
}
