/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/trace/parser/ParsedIoTraceEventQueue.h>

#include <octf/utils/Semaphore.h>

namespace octf {

class ParsedIoTraceEventQueue::Parser : public ParsedIoTraceEventHandler {
public:
    static constexpr uint32_t QUEUE_LIMIT = 1000;

    Parser(const std::string &tracePath)
            : ParsedIoTraceEventHandler(tracePath)
            , m_finished(false)
            , m_queue()
            , m_queueLock()
            , m_queueLimit(QUEUE_LIMIT)
            , m_eventInQueue() {}
    virtual ~Parser() {
        cancel();
    }

    void handleIO(const proto::trace::ParsedEvent &io) override {
        if (!isCancelRequested()) {
            m_queueLimit.wait();
            std::lock_guard<std::mutex> guard(m_queueLock);
            m_queue.emplace(proto::trace::ParsedEvent(io));
            m_eventInQueue.notify_one();
        }
    }

    void cancel() override {
        ParsedIoTraceEventHandler::cancel();
        m_queueLimit.post();
    }

    bool empty() {
        // Wait for event in queue
        std::unique_lock<std::mutex> lock(m_queueLock);
        m_eventInQueue.wait(lock,
                            [this] { return m_queue.size() || m_finished; });

        return m_finished && m_queue.empty();
    }

    const proto::trace::ParsedEvent &front() const {
        // Wait for event in queue
        std::unique_lock<std::mutex> lock(m_queueLock);
        m_eventInQueue.wait(lock,
                            [this] { return m_queue.size() || m_finished; });

        if (m_queue.empty()) {
            throw Exception("Accessing empty parsed IO queue");
        }

        return m_queue.front();
    }

    void pop() {
        // Wait for event in queue
        std::unique_lock<std::mutex> lock(m_queueLock);
        m_eventInQueue.wait(lock,
                            [this] { return m_queue.size() || m_finished; });

        if (m_queue.empty()) {
            throw Exception("Trying pop empty parsed IO queue");
        }

        m_queueLimit.post();
        m_queue.pop();
    }

    void processEvents() override {
        ParsedIoTraceEventHandler::processEvents();

        std::lock_guard<std::mutex> guard(m_queueLock);
        m_finished = true;
        m_eventInQueue.notify_one();
    }

private:
    bool m_finished;
    std::queue<proto::trace::ParsedEvent> m_queue;
    mutable std::mutex m_queueLock;
    Semaphore m_queueLimit;
    mutable std::condition_variable m_eventInQueue;
};

ParsedIoTraceEventQueue::ParsedIoTraceEventQueue(const std::string &tracePath)
        : m_parser(new ParsedIoTraceEventQueue::Parser(tracePath))
        , m_thread()
        , m_exception("") {
    m_thread = std::thread([this]() {
        try {
            m_parser->processEvents();
        } catch (Exception &e) {
            m_exception = e;
        }
    });
}

ParsedIoTraceEventQueue::~ParsedIoTraceEventQueue() {
    m_parser->cancel();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

const proto::trace::ParsedEvent &ParsedIoTraceEventQueue::front() {
    if (isException()) {
        throwException();
    }

    return m_parser->front();
}

bool ParsedIoTraceEventQueue::empty() {
    if (isException()) {
        throwException();
    }

    return m_parser->empty();
}

void ParsedIoTraceEventQueue::pop() {
    if (isException()) {
        throwException();
    }

    m_parser->pop();
}

bool ParsedIoTraceEventQueue::isException() {
    return m_exception.getMessage() != "";
}

void ParsedIoTraceEventQueue::throwException() {
    throw m_exception;
}

}  // namespace octf
