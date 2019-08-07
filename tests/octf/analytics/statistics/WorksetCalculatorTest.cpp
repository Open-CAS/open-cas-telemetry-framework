/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <gtest/gtest.h>
#include <limits>
#include <random>
#include <vector>
#include <octf/analytics/statistics/WorksetCalculator.h>

using namespace octf;

TEST(WorksetCalculator, exclusiveRanges) {
    WorksetCalculator wc;

    int ranges = 100000;
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

    int ranges = 100000;
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

    int ranges = 100000;
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

    int ranges = 100000;
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

    int ranges = 100000;
    int range = 0;

    int len = 10;
    int begin = 0;

    // Add adjacent ranges like so:
    // [0-10], [10-20], [20-30], ...
    for (; range < ranges; begin += (len - 1), range++) {
        wc.insertRange(begin, len);
    }

    ASSERT_EQ(wc.getWorkset(), ranges * len - ranges * 1 + 1);
}

TEST(WorksetCalculator, overlappingRanges) {
    WorksetCalculator wc;

    int ranges = 100000;
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
    // Add lots of random ranges, then add one maximum range
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
