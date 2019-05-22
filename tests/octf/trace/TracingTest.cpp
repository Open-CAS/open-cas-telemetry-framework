/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <gtest/gtest.h>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <random>
#include <thread>
#include <vector>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>

#include <octf/trace/internal/trace_env_usr.h>
#include <octf/trace/trace.h>

#ifdef __cplusplus
}
#endif

std::mt19937 randomGenerator(time(nullptr));
std::uniform_int_distribution<uint64_t> randomDistribution(0, UINT64_MAX);

/**
 * @test The test purpose is validate trace functionality. Test cases consists
 * of two ends. The first one pushes Event objects into trace. The second one
 * consumes event objects and check integrity of written and read event objects.
 */

/**
 * @class The class representing object pushed into trace and poped from trace.
 */
class Event {
public:
    typedef uint64_t EventSeqId;

    Event()
            : m_buffer(nullptr)
            , m_size(0) {
        m_size = sizeof(EventSeqId);
        m_buffer = new char[m_size];
        setSeqId();
    }

    Event(uint32_t size)
            : m_buffer(nullptr)
            , m_size(size) {
        if (m_size < sizeof(EventSeqId)) {
            m_size = sizeof(EventSeqId);
        }

        m_buffer = new char[m_size];

        for (uint64_t i = 0; i < m_size; i++) {
            m_buffer[i] = randomDistribution(randomGenerator);
        }

        setSeqId();
    }

    Event(void *buffer, uint32_t size)
            : m_buffer(nullptr)
            , m_size(size) {
        m_buffer = new char[m_size];
        memcpy(m_buffer, buffer, m_size);
    }

    Event(const Event &e)
            : m_buffer(nullptr)
            , m_size(e.m_size) {
        m_buffer = new char[m_size];
        memcpy(m_buffer, e.m_buffer, m_size);
    }

    Event &operator=(const Event &e) {
        if (m_buffer) {
            delete[] m_buffer;
        }

        m_size = e.m_size;
        m_buffer = new char[m_size];
        memcpy(m_buffer, e.m_buffer, m_size);

        return *this;
    }

    virtual ~Event() {
        delete[] m_buffer;
    }

    void *getBuffer() {
        return m_buffer;
    }

    uint32_t getBufferSize() {
        return m_size;
    }

    bool operator==(const Event &e) const {
        if (this == &e) {
            return true;
        }

        if (this->m_size != e.m_size) {
            return false;
        }

        if (memcmp(this->m_buffer, e.m_buffer, m_size)) {
            return false;
        }

        return true;
    }

    uint64_t getSeqId() {
        uint64_t *id = reinterpret_cast<uint64_t *>(m_buffer);
        return *id;
    }

private:
    void setSeqId() {
        static env_atomic64 seq_id_ref;

        uint64_t *id = reinterpret_cast<uint64_t *>(m_buffer);

        *id = env_atomic64_inc_return(&seq_id_ref);
    }

private:
    char *m_buffer;
    uint32_t m_size;
};

class TracingTest : public testing::Test {
public:
    TracingTest()
            : m_buffer(BUFFER_SIZE)
            , m_traceConsumer()
            , m_traceProducer() {}

    virtual ~TracingTest() = default;

protected:
    virtual void SetUp() override {
        int result;

        result = octf_trace_open(getBuffer(), getBufferSize(), NULL,
                                 octf_trace_open_mode_producer,
                                 &m_traceProducer);
        ASSERT_EQ(result, 0);

        result = octf_trace_open(getBuffer(), getBufferSize(), NULL,
                                 octf_trace_open_mode_consumer,
                                 &m_traceConsumer);
        ASSERT_EQ(result, 0);
    }

    virtual void TearDown() override {
        int result;

        // Close producer trace
        octf_trace_close(&m_traceProducer);

        EXPECT_EQ(1, octf_trace_is_closed(m_traceConsumer));

        // Try pop event from closed trace
        char event[MAX_EVENT_SIZE];
        uint32_t size = sizeof(event);
        result = octf_trace_pop(m_traceConsumer, event, &size);
        ASSERT_EQ(result, -EBADF);

        // Close consumer trace
        octf_trace_close(&m_traceConsumer);
    }

    virtual char *getBuffer() {
        return m_buffer.data();
    }

    virtual uint32_t getBufferSize() {
        return m_buffer.size();
    }

protected:
    vector<char> m_buffer;
    octf_trace_t m_traceConsumer;
    octf_trace_t m_traceProducer;

    static constexpr uint32_t BUFFER_SIZE = OCTF_TRACE_MIN_BUFFER_SIZE;
    static constexpr uint32_t MAX_EVENT_SIZE = 32;
};

/**
 * @test This test case first pushes event, then pops it. At the end of tests
 * there is the check of events equality.
 */
TEST_F(TracingTest, PushOnePopOne) {
    int result;
    int i = 0;

    list<Event> pushedEvents, readEvents;

    for (i = 0; i < BUFFER_SIZE / MAX_EVENT_SIZE * 2048; i++) {
        uint32_t size = randomDistribution(randomGenerator) % MAX_EVENT_SIZE;
        Event e(size);

        result = octf_trace_push(m_traceProducer, e.getBuffer(),
                                 e.getBufferSize());
        if (0 == result) {
            pushedEvents.push_back(e);

            EXPECT_EQ(0, octf_trace_is_empty(m_traceProducer));
            EXPECT_EQ(0, octf_trace_is_empty(m_traceConsumer));
        }

        char content[MAX_EVENT_SIZE];
        size = sizeof(content);

        result = octf_trace_pop(m_traceConsumer, content, &size);
        if (0 == result) {
            Event e(content, size);
            readEvents.push_back(e);
        } else {
            FAIL();
        }
    }

    // Check if number of events is equal in input and output list
    ASSERT_EQ(pushedEvents.size(), readEvents.size());

    // Check content of events
    EXPECT_TRUE(pushedEvents == readEvents);

    EXPECT_EQ(1, octf_trace_is_empty(m_traceProducer));
    EXPECT_EQ(1, octf_trace_is_empty(m_traceConsumer));
}

/**
 * @test This test first push events until traces is full. Then pops events
 * until trace is empty. This procedure is repeated few times.
 * At the end of test test checks if written events are equal read ones.
 */
TEST_F(TracingTest, PushManyPopMany) {
    int result;
    int i = 0;

    list<Event> pushedEvents, poppedEvents;

    for (i = 0; i < 2048; i++) {
        // fill log
        do {
            uint64_t size =
                    randomDistribution(randomGenerator) % MAX_EVENT_SIZE;
            Event e(size);

            result = octf_trace_push(m_traceProducer, e.getBuffer(),
                                     e.getBufferSize());
            if (0 == result) {
                pushedEvents.push_back(e);
            }

        } while (!result);

        EXPECT_EQ(0, octf_trace_is_empty(m_traceProducer));
        EXPECT_EQ(0, octf_trace_is_empty(m_traceConsumer));

        // read log
        do {
            char content[MAX_EVENT_SIZE];
            uint32_t size = sizeof(content);

            result = octf_trace_pop(m_traceConsumer, content, &size);
            if (0 == result) {
                Event e(content, size);
                poppedEvents.push_back(e);
            }

        } while (!result);

        // Check if number events is equal in input and output list
        ASSERT_EQ(pushedEvents.size(), poppedEvents.size());

        EXPECT_TRUE(pushedEvents == poppedEvents);

        EXPECT_EQ(1, octf_trace_is_empty(m_traceProducer));
        EXPECT_EQ(1, octf_trace_is_empty(m_traceConsumer));

        pushedEvents.clear();
        poppedEvents.clear();
    }
}

class TestEventLog {
public:
    TestEventLog()
            : m_events()
            , m_lock() {}

    void push(Event &event) {
        int result = -1;

        m_lock.lock();
        if (m_events.end() == m_events.find(event.getSeqId())) {
            m_events[event.getSeqId()] = event;
            result = 0;
        }
        m_lock.unlock();

        ASSERT_EQ(result, 0);  // Duplicated event
    }

    bool pop(Event &event) {
        bool result = false;
        map<uint64_t, Event>::iterator iter;

        m_lock.lock();

        iter = m_events.find(event.getSeqId());
        if (m_events.end() != iter) {
            EXPECT_TRUE(event == iter->second);
            m_events.erase(iter);
            result = true;
        }

        m_lock.unlock();

        return result;
    }

private:
    map<uint64_t, Event> m_events;
    mutex m_lock;
};

class Worker {
public:
    Worker(octf_trace_t log, TestEventLog &eventLog)
            : m_log(log)
            , m_thread()
            , m_eventLog(eventLog)
            , m_run() {}

    virtual ~Worker() = default;

    void stop() {
        env_atomic64_set(&m_run, 0);
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

    void start() {
        env_atomic64_set(&m_run, 1);
        resetEventCount();

        auto threadFn = [this]() {
            work();
            env_atomic64_set(&m_run, 0);
        };

        m_thread = thread(threadFn);
    }

    bool isTestRunning() {
        return 1 == env_atomic64_read(&m_run);
    }

    static void resetEventCount() {
        env_atomic64_set(&m_eventCount, 0);
    }

protected:
    void terminate() {
        env_atomic64_set(&m_run, 0);
    }

private:
    virtual void work() = 0;

protected:
    octf_trace_t m_log;
    thread m_thread;
    TestEventLog &m_eventLog;
    env_atomic64 m_run;
    static constexpr int64_t MAX_EVENTS_COUNT = 1000UL * 100UL;
    static constexpr int64_t MAX_EVENTS_SIZE = 1000UL;
    static env_atomic64 m_eventCount;
};

env_atomic64 Worker::m_eventCount;

class EventWriter : public Worker {
public:
    EventWriter(octf_trace_t log, TestEventLog &eventLog)
            : Worker(log, eventLog) {}

    virtual ~EventWriter() {}

private:
    void work() {
        while (isTestRunning() && !isEventLimitReached()) {
            uint32_t size =
                    randomDistribution(randomGenerator) % MAX_EVENTS_SIZE;
            Event e(size);

            m_eventLog.push(e);

            int result =
                    octf_trace_push(m_log, e.getBuffer(), e.getBufferSize());

            if (0 == result) {
                env_atomic64_inc(&m_eventCount);
            }
        }
    }

private:
    bool isEventLimitReached() const {
        if (env_atomic64_read(&m_eventCount) > MAX_EVENTS_COUNT) {
            return true;
        } else {
            return false;
        }
    }
};

class EventReader : public Worker {
public:
    EventReader(octf_trace_t log, TestEventLog &eventLog)
            : Worker(log, eventLog) {}

    virtual ~EventReader() {}

private:
    void work() {
        while (isTestRunning()) {
            process();
        }

        while (1 != octf_trace_is_empty(m_log)) {
            // Fetch left events
            process();
        }
    }

public:
    void process() {
        char content[MAX_EVENTS_SIZE];
        uint32_t size = sizeof(content);

        int result = octf_trace_pop(m_log, content, &size);
        if (result) {
            return;
        }

        Event e(content, size);

        if (!m_eventLog.pop(e)) {
            terminate();
            FAIL();
        }
    }
};

/**
 * @class Base class for multi-thread tests defined further. In writer workers
 * event are written, and in reader workers events are read. Each time,
 * when event is read, there is the check of event integrity.
 */
class TracingMultiThreadTest : public TracingTest {
public:
    TracingMultiThreadTest()
            : TracingTest()
            , m_eventLog()
            , m_writers()
            , m_readers()
            , m_writersNo(0)
            , m_readersNo(0)
            , m_buffer(BUFFER_SIZE) {}

    void run(uint8_t writersNo, uint8_t readersNo) {
        m_writersNo = writersNo;
        m_readersNo = readersNo;

        createWorkers();
        startWorkers();

        while (isTestRunning()) {
            usleep(1000 * 10);
        }

        stopWorkers();
        destroyWorkers();
    }

private:
    bool isTestRunning() {
        for (auto &wr : m_writers) {
            if (!wr->isTestRunning()) {
                return false;
            }
        }

        for (auto &rd : m_readers) {
            if (!rd->isTestRunning()) {
                return false;
            }
        }

        return true;
    }

    void createWorkers() {
        for (uint64_t i = 0; i < m_writersNo; i++) {
            m_writers.push_back(new EventWriter(m_traceProducer, m_eventLog));
        }

        for (uint64_t i = 0; i < m_readersNo; i++) {
            m_readers.push_back(new EventReader(m_traceConsumer, m_eventLog));
        }
    }

    void destroyWorkers(void) {
        auto iter_wr = m_writers.begin();

        for (; iter_wr != m_writers.end();) {
            delete *iter_wr;
            iter_wr = m_writers.erase(iter_wr);
        }

        auto iter_rd = m_readers.begin();

        for (; iter_rd != m_readers.end();) {
            delete *iter_rd;
            iter_rd = m_readers.erase(iter_rd);
        }
    }

    void startWorkers(void) {
        for (auto &writer : m_writers) {
            writer->start();
        }

        for (auto &reader : m_readers) {
            reader->start();
        }
    }

    void stopWorkers(void) {
        for (auto &writer : m_writers) {
            writer->stop();
        }

        for (auto &reader : m_readers) {
            reader->stop();

            // We have to flush trace, wait until readers processed all events
            while (0 == octf_trace_is_empty(m_traceConsumer)) {
                reader->process();
            }
        }
    }

private:
    virtual char *getBuffer() override {
        return m_buffer.data();
    }

    virtual uint32_t getBufferSize() override {
        return m_buffer.size();
    }

private:
    TestEventLog m_eventLog;
    list<EventWriter *> m_writers;
    list<EventReader *> m_readers;
    uint8_t m_writersNo;
    uint8_t m_readersNo;
    vector<char> m_buffer;
    static constexpr uint32_t BUFFER_SIZE = 1024 * 1024;
};

TEST_F(TracingMultiThreadTest, PushManyPopMany) {
    run(1, 1);
}

TEST_F(TracingMultiThreadTest, ManyProducerOneConsumer) {
    run(16, 1);
}

TEST_F(TracingMultiThreadTest, OneProducerManyConsumer) {
    run(1, 16);
}

TEST_F(TracingMultiThreadTest, ManyProducerManyConsumer) {
    run(16, 16);
}
