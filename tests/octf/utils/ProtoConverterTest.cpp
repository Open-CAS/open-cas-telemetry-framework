/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <gtest/gtest.h>
#include <limits>
#include <list>
#include <ostream>
#include <octf/utils/ProtoConverter.h>

using namespace octf::protoconverter;
using namespace std;

TEST(ProtoConverter, VarintEncodeDecodeIntegrity) {
    uint8_t buffer[MAX_VARINT32_BYTES];

    // Initialize random seed
    srand(time(NULL));

    // Encode and decode numbers spanning acrross all int range
    for (int64_t inValue = numeric_limits<int>::min();
         inValue <= numeric_limits<int>::max(); inValue += rand() % 1000) {
        int inSize;
        int outSize;
        int outValue;

        inSize = encodeVarint32(buffer, sizeof(buffer), inValue);
        outSize = decodeVarint32(buffer, sizeof(buffer), outValue);

        ASSERT_GT(inSize, 0);
        ASSERT_GT(outSize, 0);
        ASSERT_EQ(inSize, outSize);
        ASSERT_EQ(inValue, outValue);
    }
}

TEST(ProtoConverter, VarintEncodeDecodeIntegrityMany) {
    uint8_t buffer[10000];

    // Initialize random seed
    srand(time(NULL));

    for (int i = 0; i < 10000; i++) {
        list<int> inSizeList;
        list<int> inValueList;

        int inSize;
        int inValue;
        int outSize;
        int outValue;

        uint8_t *bufferPtr = buffer;
        uint64_t bufferSize = sizeof(buffer);

        do {
            inValue = rand();
            if (rand() % 2) {
                inValue *= -1;
            }

            inSize = encodeVarint32(bufferPtr, bufferSize, inValue);
            ASSERT_GE(inSize, 0);

            if (inSize) {
                inSizeList.push_back(inSize);
                inValueList.push_back(inValue);

                bufferPtr += inSize;
                bufferSize -= inSize;
            }
        } while (inSize);

        bufferPtr = buffer;
        bufferSize = sizeof(buffer);

        do {
            outSize = decodeVarint32(bufferPtr, bufferSize, outValue);
            ASSERT_GE(outSize, 0);

            if (outSize) {
                bufferPtr += outSize;
                bufferSize -= outSize;
            } else {
                break;
            }

            ASSERT_GT(inSizeList.size(), 0);
            ASSERT_EQ(outSize, inSizeList.front());
            inSizeList.pop_front();

            ASSERT_GT(inValueList.size(), 0);
            ASSERT_EQ(outValue, inValueList.front());
            inValueList.pop_front();
        } while (outSize);

        ASSERT_TRUE(inSizeList.empty());
        ASSERT_TRUE(inValueList.empty());
    }
}
