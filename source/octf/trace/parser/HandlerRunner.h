/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
#define SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H

#include <functional>
#include <list>
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
 * @tparam Event template type of event handled by handler
 */
template <typename Handler, typename Event>
class HandlerRunner : NonCopyable {
    static_assert(std::is_base_of<TraceEventHandler<Event>, Handler>(),
                  "Attempted to instantiate template with wrong event type.");

public:
    HandlerRunner()
            : NonCopyable()
            , m_lock()
            , m_funcs()
            , m_jobs() {}
    virtual ~HandlerRunner() = default;

    /**
     * @brief Completion callback invoked when the handle has ended its job
     *
     * @param hndlr Shared reference to the handler which has finished its job
     *
     * @note Callbacks are synchronized
     */
    typedef std::function<void(std::shared_ptr<Handler> hndlr)>
            CompletionCallback;

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
     * @param cmpl Completion callback called when a handler will end
     * @param error Error handler called when an error will occur
     * @param args Arguments used for creation handler
     *
     * @code
     *
     * // Declare your runner for given handler
     * using Handler = TraceEventHandlerStats;
     * octf::HandlerRunner<Handler, Handler::eventType> runner;
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
     * runner.addHandler(cmpl, error, tracepath,
     *                      otherRequiredArgumentsToCreateHandler_1);
     * runner.addHandler(cmpl, error, tracepath,
     *                      otherRequiredArgumentsToCreateHandler_2);
     *
     * // Run all handlers concurrently
     * runner.run();
     *
     * // All done, please remember to check if error occurred
     * // Good luck!!!
     * @endcode
     */
    template <typename... Args>
    void addHandler(CompletionCallback cmplCallback,
                    ErrorCallback errorCallback,
                    Args... args) {
        auto func = [this, cmplCallback, errorCallback, args...]() {
            try {
                ResourcesGuarder rGuarder(0.75);
                rGuarder.lock();  // Grab resources lock to be sure not running
                                  // out of resources

                {
                    auto handler = std::make_shared<Handler>(args...);
                    handler->processEvents();
                    complete(cmplCallback, handler);
                }

                // Release resources
                rGuarder.unlock();
            } catch (Exception &e) {
                error(errorCallback, e.getMessage());
            } catch (std::exception &e) {
                error(errorCallback, e.what());
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
    void complete(CompletionCallback cmpl, std::shared_ptr<Handler> handler) {
        std::lock_guard<std::mutex> guard(m_lock);
        cmpl(handler);
    }

    void error(ErrorCallback error, const std::string &message) {
        std::lock_guard<std::mutex> guard(m_lock);
        error(message);
    }

private:
    std::mutex m_lock;
    std::list<std::function<void()>> m_funcs;
    std::list<std::thread> m_jobs;
};

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_PARSER_HANDLERRUNNER_H
