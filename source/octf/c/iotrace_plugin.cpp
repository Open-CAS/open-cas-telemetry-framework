/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/c/iotrace_plugin.h>

#include <chrono>
#include <octf/trace/IOTracePlugin.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>

using namespace octf;

class IOTracePluginC : public IOTracePlugin {
public:
    IOTracePluginC(octf_iotrace_plugin_context_t context,
                   const std::string &pluginId,
                   uint32_t queueCount)
            : IOTracePlugin(pluginId, queueCount)
            , m_context(context) {}

    virtual ~IOTracePluginC() = default;

    bool startTrace() override {
        bool result = IOTracePlugin::startTrace();

        if (result) {
            m_context->tracing_active = true;
        }

        return result;
    }

    bool stopTrace() override {
        m_context->tracing_active = false;
        return IOTracePlugin::stopTrace();
    }

private:
    octf_iotrace_plugin_context_t m_context;
};

extern "C" int octf_iotrace_plugin_create(
        const struct octf_iotrace_plugin_cnfg *cnfg,
        octf_iotrace_plugin_context_t *_context) {
    IOTracePluginC *plugin = NULL;
    auto context = new struct octf_iotrace_plugin_context;

    if (nullptr == context) {
        return -ENOMEM;
    }

    try {
        const char *prefix = "[OCTF]";
        log::cout << log::prefix << prefix;
        log::debug << log::prefix << prefix;
        log::verbose << log::prefix << prefix;
        log::cerr << log::prefix << prefix;
        log::critical << log::prefix << prefix;

        context->ref_sid = 0;
        context->tracing_active = false;
        plugin = new IOTracePluginC(context, cnfg->id, cnfg->io_queue_count);
        context->plugin = plugin;

        if (!plugin->init()) {
            delete plugin;
            delete context;
            return -EINVAL;
        }

    } catch (Exception &e) {
        log::cerr << e.what() << std::endl;
        if (context->plugin) {
            delete plugin;
            delete context;
            return -EINVAL;
        }
    }

    log::cout << "Plugin " << plugin->getNodeId().getId() << " started"
              << std::endl;

    *_context = context;
    return 0;
}

extern "C" void octf_iotrace_plugin_destroy(
        octf_iotrace_plugin_context_t *_context) {
    if (NULL == _context) {
        return;
    }

    octf_iotrace_plugin_context_t context = *_context;
    if (NULL == context) {
        return;
    }

    auto plugin = static_cast<IOTracePluginC *>(context->plugin);

    if (plugin) {
        try {
            plugin->deinit();
        } catch (Exception &e) {
            log::cerr << e.what() << std::endl;
        }

        delete plugin;
    }

    delete context;
    *_context = NULL;
}

extern "C" void octf_iotrace_plugin_init_trace_header(
        octf_iotrace_plugin_context_t context,
        struct iotrace_event_hdr *hdr,
        iotrace_event_type type,
        uint32_t size) {
    if (context) {
        using namespace std::chrono;
        auto timestamp =
                high_resolution_clock::now().time_since_epoch().count();

        uint64_t sid =
                __atomic_add_fetch(&context->ref_sid, 1, __ATOMIC_SEQ_CST);

        iotrace_event_init_hdr(hdr, type, sid, timestamp, size);
    }
}

extern "C" void octf_iotrace_plugin_push_trace(
        octf_iotrace_plugin_context_t context,
        uint32_t ioQueue,
        const void *trace,
        size_t size) {
    if (context) {
        auto plugin = static_cast<IOTracePluginC *>(context->plugin);

        if (plugin) {
            plugin->push(ioQueue, trace, size);
        }
    }
}
