/*
 * Copyright(c) 2012-2018 Intel Corporation
 * Copyright 2023 Solidigm All Rights Reserved
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/c/iotrace_plugin.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <octf/trace/IOTracePlugin.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/cli/Executor.h>
#include <octf/interface/InterfaceConfigurationImpl.h>
#include <octf/interface/InterfaceTraceManagementImpl.h>
#include <octf/interface/InterfaceTraceParsingImpl.h>
#include <octf/utils/Exception.h>

using namespace octf;

class IOTracePluginC : public IOTracePlugin {
private:
    struct QueueContext {
        std::atomic_bool traceStopping;
        std::atomic<uint64_t> tracingRefCounter;
    };

public:
    IOTracePluginC(octf_iotrace_plugin_context_t context,
                   const std::string &pluginId,
                   uint32_t queueCount)
            : IOTracePlugin(pluginId, queueCount)
            , m_context(context)
            , m_queueContext(queueCount) {
        for (auto &queueContext : m_queueContext) {
            queueContext.tracingRefCounter = 0;
            queueContext.traceStopping = true;
        }
    }

    virtual ~IOTracePluginC() = default;

    bool startTrace() override {
        bool result = IOTracePlugin::startTrace();

        if (result) {
            m_context->tracing_active = true;
            for (auto &queueContext : m_queueContext) {
                queueContext.traceStopping = false;
            }
        }

        return result;
    }

    bool stopTrace() override {
        m_context->tracing_active = false;

        for (auto &queueContext : m_queueContext) {
            queueContext.traceStopping = true;
        }

        // Poll for all ongoing traces completion
        while (isTraceOngoing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        return IOTracePlugin::stopTrace();
    }

    void push(uint32_t ioQueueId, const void *trace, size_t size) override {
        if (ioQueueId > getTraceQueueCount()) {
            ioQueueId %= getTraceQueueCount();
        }

        auto &queueContext = m_queueContext[ioQueueId];

        queueContext.tracingRefCounter++;
        if (queueContext.traceStopping) {
            // Tracing stop was requested
            queueContext.tracingRefCounter--;
            return;
        }
        IOTracePlugin::push(ioQueueId, trace, size);
        queueContext.tracingRefCounter--;
    }

private:
    inline bool isTraceOngoing() const {
        for (const auto &queueContext : m_queueContext) {
            if (queueContext.tracingRefCounter) {
                return true;
            }
        }

        return false;
    }

private:
    octf_iotrace_plugin_context_t m_context;
    std::vector<QueueContext> m_queueContext;
};

extern "C" int octf_iotrace_plugin_create(
        const struct octf_iotrace_plugin_cnfg *cnfg,
        octf_iotrace_plugin_context_t *_context) {
    IOTracePluginC *plugin = nullptr;
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
        if (nullptr == plugin) {
            delete context;
            return -ENOMEM;
        }

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
    if (nullptr == _context) {
        return;
    }

    octf_iotrace_plugin_context_t context = *_context;
    if (nullptr == context) {
        return;
    }

    auto plugin = static_cast<IOTracePluginC *>(context->plugin);

    if (plugin) {
        NodeId id = plugin->getNodeId();

        try {
            plugin->deinit();
        } catch (Exception &e) {
            log::cerr << e.what() << std::endl;
        }

        delete plugin;

        log::cout << "Plugin " << id.getId() << " stopped" << std::endl;
    }

    delete context;
    *_context = nullptr;
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

extern "C" int octf_iotrace_plugin_main(const char *app_name,
                                        const char *app_version,
                                        const char *id,
                                        int argc,
                                        char *argv[]) {

    try {
        // Create executor and local command set
        cli::Executor ex;

        auto &properties = ex.getCliProperties();

        properties.setName(app_name);
        properties.setVersion(app_version);

        // Create interfaces
        ex.addModules(NodeId(id));

        // Trace Management Interface
        InterfaceShRef iTraceManagement =
                std::make_shared<InterfaceTraceManagementImpl>("");

        // Trace Parsing Interface
        InterfaceShRef iTraceParsing =
                std::make_shared<InterfaceTraceParsingImpl>();

        // Configuration Interface for setting trace repository path
        InterfaceShRef iConfiguration =
                std::make_shared<InterfaceConfigurationImpl>();

        // Add interfaces to executor
        ex.addModules(iTraceManagement, iTraceParsing, iConfiguration);

        // Execute command
        return ex.execute(argc, argv);

    } catch (Exception &e) {
        log::cerr << e.what() << std::endl;
        return 1;
    } catch (std::exception &e) {
        log::critical << app_name << " execution interrupted: " << e.what()
                      << std::endl;
        return 1;
    }
}
