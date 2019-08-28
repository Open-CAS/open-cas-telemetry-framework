/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#ifndef SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
#define SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H

#include <list>
#include <memory>
#include <thread>
#include <octf/trace/parser/TraceEventHandler.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Semaphore.h>

namespace octf {

/**
 * @brief Utility class which allows to run many handlers concurrently
 *
 * @tparam Handler type of handler to be run
 * @tparam Event template type of event handled by handler
 */
template <typename Handler, typename Event>
class HandlerRunner {
    static_assert(std::is_base_of<TraceEventHandler<Event>, Handler>(),
                  "Attempted to instantiate template with wrong event type.");

public:
    HandlerRunner() = default;
    virtual ~HandlerRunner() = default;

    /**
     * @brief Adds handler to runner
     * @param handler Handler to be run
     */
    void addHandler(std::shared_ptr<Handler> handler) {
        m_handlers.push_back(handler);
    }

    // TODO (trybicki) Limit thread number based on available cpus and/or memory
    /**
     * @brief Runs all added handlers
     */
    void run() {
        for (auto &handler : m_handlers) {
            m_jobs.emplace_back(
                    std::thread([&]() { handler->processEvents(); }));
        }

        for (auto &job : m_jobs) {
            job.join();
        }

        m_finished = true;
    }

    /**
     * @brief Gets a list of handlers which finished processing
     *
     * @note Call this function only after run() has finished
     */
    std::list<std::shared_ptr<Handler>> &getHandlers() {
        if (m_finished) {
            return m_handlers;
        }

        throw Exception("Called getHandlers() before they finished");
    }

private:
    bool m_finished;
    std::list<std::shared_ptr<Handler>> m_handlers;
    std::list<std::thread> m_jobs;
    Semaphore m_sem;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
