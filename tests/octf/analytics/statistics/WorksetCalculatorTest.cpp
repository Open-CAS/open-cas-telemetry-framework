/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>
#include <limits>
#include <random>
#include <vector>
#include <octf/analytics/statistics/WorksetCalculator.h>

using namespace octf;

TEST(WorksetCalculator, exclusiveRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 10;
    int begin = 0;

    // Add exclusive ranges like so:
    // [0-10], [20-30], [40-50], ...
    for (; range < ranges; begin += len + 10, range++) {
        wc.insertRange(begin, len);
    }

    ASSERT_EQ(wc.getWorkset(), ranges * len);
}

TEST(WorksetCalculator, identicalRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 10;
    int begin = 0;

    for (; range < ranges; range++) {
        wc.insertRange(begin, len);
    }

    ASSERT_EQ(wc.getWorkset(), len);
}

TEST(WorksetCalculator, subRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int maxLen = ranges * 2 + 2;
    int currentLen = maxLen;
    int currentBegin = 0;

    // Insert the biggest range
    wc.insertRange(currentBegin, maxLen);

    // Add subranges like so:
    // First range: [0-100]
    // Next ranges: [1-99], [2-98], ...
    for (; range < ranges; range++) {
        currentBegin++;
        currentLen--;
        assert(currentLen > 0);

        wc.insertRange(currentBegin, currentLen);
    }

    ASSERT_EQ(wc.getWorkset(), maxLen);
}

TEST(WorksetCalculator, subRanges2) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int currentLen = 1;
    int currentBegin = ranges;

    // Add subranges like so:
    // First range: [49-51]
    // Next ranges: [48-52], [47-53], ... , [0-100]
    for (; range < ranges; range++) {
        assert(currentBegin >= 0);

        wc.insertRange(currentBegin, currentLen);
        currentBegin--;
        currentLen++;
    }

    ASSERT_EQ(wc.getWorkset(), currentLen - 1);
}

TEST(WorksetCalculator, adjacentRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 10;
    int begin = 0;

    // Add adjacent ranges like so:
    // [0-10], [10-20], [20-30], ...
    for (; range < ranges; begin += len, range++) {
        wc.insertRange(begin, len);
    }

    ASSERT_EQ(wc.getWorkset(), ranges * len);
}

TEST(WorksetCalculator, overlappingRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 11;
    int begin = 0;

    // Overlap ranges like so:
    // [0-11], [10-21], [20-31], ...
    for (; range < ranges; begin += (len - 1), range++) {
        wc.insertRange(begin, len);
    }

    ASSERT_EQ(wc.getWorkset(), ranges * len - ranges * 1 + 1);
}

TEST(WorksetCalculator, randomRanges) {
    // Add lots of random ranges with random length, then add one maximum range
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> rand(
            1, std::numeric_limits<int>::max());

    WorksetCalculator wc;

    int ranges = 100000;
    int range = 0;

    uint64_t len;
    uint64_t begin;

    for (; range < ranges; range++) {
        begin = rand(rng) - 1;
        len = rand(rng);
        wc.insertRange(begin, len);
    }

    begin = 0;
    len = std::numeric_limits<uint64_t>::max();
    wc.insertRange(begin, len);

    ASSERT_EQ(wc.getWorkset(), len);
}

TEST(WorksetCalculator, randomRanges2) {
    // Add lots of random ranges, with predefined short length
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> rand(
            1, std::numeric_limits<int>::max());

    WorksetCalculator wc;

    int ranges = 100000;
    int range = 0;

    uint64_t len = 4096;
    uint64_t begin;

    for (; range < ranges; range++) {
        begin = rand(rng) - 1;
        wc.insertRange(begin, len);
    }

    begin = 0;
    len = std::numeric_limits<uint64_t>::max();
    wc.insertRange(begin, len);

    ASSERT_EQ(wc.getWorkset(), len);
}

TEST(WorksetCalculator, removeRanges) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 11;
    int begin = 0;

    // Overlap ranges like so:
    // [0-11], [10-21], [20-31], ...
    for (; range < ranges; begin += (len - 1), range++) {
        wc.insertRange(begin, len);
    }

    uint64_t worksetAfterInserts = ranges * len - ranges * 1 + 1;
    ASSERT_EQ(wc.getWorkset(), worksetAfterInserts);

    // Remove every second range:
    // [0-11], [20-31], ...
    range = 0;
    begin = 0;
    for (; range < ranges; begin += 2 * (len - 1), range += 2) {
        wc.insertRange(begin, len);
    }

    // Workset should be the same
    ASSERT_EQ(wc.getWorkset(), worksetAfterInserts);

    // Add ranges again
    range = 0;
    begin = 0;
    for (; range < ranges; begin += (len - 1), range++) {
        wc.insertRange(begin, len);
    }

    // Workset should be the same
    ASSERT_EQ(wc.getWorkset(), worksetAfterInserts);
}

TEST(WorksetCalculator, removeRanges2) {
    WorksetCalculator wc;

    int ranges = 10000;
    int range = 0;

    int len = 11;
    int begin = 0;

    // Overlap ranges like so:
    // [0-11], [10-21], [20-31], ...
    // And remove them right after adding
    for (; range < ranges; begin += (len - 1), range++) {
        wc.insertRange(begin, len);
        wc.removeRange(begin, len);
    }

    uint64_t workset = len;

    // Workset should be equal to just one range
    ASSERT_EQ(wc.getWorkset(), workset);
}

TEST(WorksetCalculator, removeAndSplitRanges) {
    WorksetCalculator wc;

    // Add [0;100] and [200;300]
    wc.insertRange(0, 100);
    wc.insertRange(200, 100);

    // Remove [50;250]
    wc.removeRange(50, 200);

    uint64_t workset = 200;
    ASSERT_EQ(wc.getWorkset(), workset);

    // Insert [100;210]
    wc.insertRange(100, 110);
    workset = 210;
    ASSERT_EQ(wc.getWorkset(), workset);
}
