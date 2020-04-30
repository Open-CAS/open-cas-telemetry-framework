/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
#define SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <octf/trace/parser/TraceEventHandler.h>
#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>
#include <octf/utils/ResourcesGuarder.h>

namespace octf {

/**
 * @brief Utility class which allows to run many handlers concurrently
 *
 * Handler using ResourceGuarder will limit the number of concurrently run
 * handlers in order to not exceed the machine available resources (e.g. memory)
 *
 * @tparam Handler type of handler to be run
 */
template <typename Handler>
class HandlerRunner : NonCopyable {
public:
    HandlerRunner()
            : NonCopyable()
            , m_lock()
            , m_funcs()
            , m_jobs() {}
    virtual ~HandlerRunner() = default;

    typedef std::shared_ptr<Handler> HandlerShRef;

    /**
     * @brief Factory used to create handler in job thread
     *
     * @return Event handler
     */
    typedef std::function<HandlerShRef(void)> Factory;

    /**
     * @brief Completion callback invoked when the handle has ended its job
     *
     * @param hndlr Shared reference to the handler which has finished its job
     *
     * @note Callbacks are synchronized
     */
    typedef std::function<void(HandlerShRef)> CompletionCallback;

    /**
     * @brief Error callback which is invoked when an error occurs
     *
     * @param error Message corresponds to the error
     *
     * @note Callbacks are synchronized
     */
    typedef std::function<void(const std::string &error)> ErrorCallback;

    /**
     * @brief Adds handler to runner
     * @param handler Handler to be run
     */
    /**
     * @brief Add a new handler to be executed when calling HandlerRunner::run
     *
     * @param factory Handler factory
     * @param cmpl Completion callback called when a handler will end
     * @param error Error handler called when an error will occur
     *
     * @code
     *
     * // Declare your runner for given handler
     * using Handler = TraceEventHandlerStats;
     * octf::HandlerRunner<Handler> runner;
     *
     * // Define handler factory
     * auto factory = []() {
     *      return std::make_shared<Handler>();
     * }
     *
     * // Define completion callback
     * auto cmpl = [](std::shared_ptr<Handler> hndlr) {
     *      // Your code handling completion of handler
     * }
     *
     * // Define error handler
     * auto error = [&result](const std::string &error) {
     *      // Your code handling the error
     * };
     *
     * // Add handlers
     * runner.addHandler(factory, cmpl, error);
     * // Add other handlers
     *
     * // Run all handlers concurrently
     * runner.run();
     *
     * // All done, please remember to check if error occurred
     * // Good luck!!!
     * @endcode
     */
    void addHandler(Factory factory,
                    CompletionCallback cmpl,
                    ErrorCallback error) {
        auto func = [this, factory, cmpl, error]() {
            try {
                ResourcesGuarder rGuarder(0.75);
                rGuarder.lock();  // Grab resources lock to be sure not running
                                  // out of resources

                {
                    HandlerShRef handler = factory();
                    handler->processEvents();

                    std::lock_guard<std::mutex> guard(m_lock);
                    cmpl(handler);
                }

                // Release resources
                rGuarder.unlock();
            } catch (Exception &e) {
                std::lock_guard<std::mutex> guard(m_lock);
                error(e.getMessage());
            } catch (std::exception &e) {
                std::lock_guard<std::mutex> guard(m_lock);
                error(e.what());
            }
        };

        m_funcs.emplace_back(func);
    }

    /**
     * @brief Runs all added handlers
     */
    void run() {
        for (auto &func : m_funcs) {
            m_jobs.emplace_back(std::thread(func));
        }

        for (auto &job : m_jobs) {
            job.join();
        }
    }

private:
    std::mutex m_lock;
    std::list<std::function<void()>> m_funcs;
    std::list<std::thread> m_jobs;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
