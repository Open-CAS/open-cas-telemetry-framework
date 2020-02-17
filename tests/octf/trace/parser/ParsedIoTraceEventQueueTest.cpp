/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <gtest/gtest.h>
#include <third_party/safestringlib.h>
#include <limits>
#include <memory>
#include <octf/octf.h>

#include <octf/UtilsTest.h>
#include <octf/trace/TraceUtilsTest.h>

using namespace octf;
using namespace std;

static constexpr uint64_t TRACE_LENGTH = 10000;

TEST(ParsedIoTraceEventQueueTest, NotExistingTrace) {
    Exception exception("");

    try {
        ParsedIoTraceEventQueue queue("NotExistingTracePath");
    } catch (Exception &e) {
        exception = e;
    }

    if (exception.getMessage() == "") {
        FAIL();
    }
}

TEST(ParsedIoTraceEventQueueTest, EmptyTrace) {
    try {
        SetupTestOutput(test_info_);

        // Create Empty Trace
        TestTrace trace(0);
        ParsedIoTraceEventQueue queue(trace.getTraceSummary().tracepath());
        ASSERT_TRUE(queue.empty());
    } catch (Exception &e) {
        log::cerr << e.getMessage() << std::endl;
        FAIL();
    }
}

TEST(ParsedIoTraceEventQueueTest, PopTraces) {
    try {
        SetupTestOutput(test_info_);

        // Create trace
        TestTrace trace(TRACE_LENGTH);
        ParsedIoTraceEventQueue queue(trace.getTraceSummary().tracepath());

        auto &lst = trace.getIoList();

        while (!lst.empty()) {
            ASSERT_FALSE(queue.empty());

            const auto &io1 = lst.front().io();
            const auto &io2 = queue.front().io();

            ASSERT_TRUE(io1.lba() == io2.lba());
            ASSERT_TRUE(io1.len() == io2.len());
            ASSERT_TRUE(io1.operation() == io2.operation());

            lst.pop_front();
            queue.pop();
        }

    } catch (Exception &e) {
        log::cerr << e.getMessage() << std::endl;
        FAIL();
    }
}

TEST(ParsedIoTraceEventQueueTest, Cancel) {
    try {
        SetupTestOutput(test_info_);

        // Create trace
        TestTrace trace(TRACE_LENGTH);
        ParsedIoTraceEventQueue queue(trace.getTraceSummary().tracepath());

        // Wait a time until queue will reach a limit,
        // exit this branch to check if we destroy queue object properly
        std::chrono::milliseconds sleepTime(1000);
        std::this_thread::sleep_for(sleepTime);

        ASSERT_FALSE(queue.empty());

    } catch (Exception &e) {
        log::cerr << e.getMessage() << std::endl;
        FAIL();
    }
}

TEST(ParsedIoTraceEventQueueTest, AccessOnEmpty) {
    try {
        SetupTestOutput(test_info_);

        // Create trace
        TestTrace trace(TRACE_LENGTH);
        ParsedIoTraceEventQueue queue(trace.getTraceSummary().tracepath());

        // Flush queue
        while (!queue.empty()) {
            queue.pop();
        }

        // Check if exception is throw when accessing front of empty queue
        bool exception = false;
        try {
            queue.front();
        } catch (Exception &e) {
            exception = true;
        }
        ASSERT_TRUE(exception);

        // Check if exception is throw when popping empty queue
        exception = false;
        try {
            queue.pop();
        } catch (Exception &e) {
            exception = true;
        }
        ASSERT_TRUE(exception);

    } catch (Exception &e) {
        log::cerr << e.getMessage() << std::endl;
        FAIL();
    }
}
