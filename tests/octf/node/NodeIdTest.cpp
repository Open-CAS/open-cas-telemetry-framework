/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <octf/node/NodeId.h>
#include <octf/utils/Exception.h>

using namespace octf;

class TestCase {
public:
    TestCase(std::string value, bool valid)
            : m_value(value)
            , m_valid(valid) {}

    bool isValid() const {
        return m_valid;
    }

    const std::string &getValue() const {
        return m_value;
    }

private:
    std::string m_value;
    bool m_valid;
};

static const std::vector<TestCase> TestCases = {
        TestCase("", false),
        TestCase("A", true),
        TestCase("a", true),
        TestCase("-", false),
        TestCase("?", false),
        TestCase("*", false),
        TestCase("0", true),
        TestCase("9", true),
        TestCase(" ", false),
        TestCase("\t", false),
        TestCase("\n", false),
        TestCase("ABCDEFGH", true),
        TestCase("ABCDEFGH-123456", true),
        TestCase("-ABCDEFGH-123456-123456", false),
        TestCase("ABCDEFGH-12345-0-1-2-3", true),
        TestCase("0-1-2-3-ABCDEFGH-12345", true),
        TestCase("-0-1-2-3-ABCDEFGH-12345-123456", false),
};

TEST(NodeId, NodeIdValidation) {
    for (const auto &id : TestCases) {
        bool result = true;

        try {
            NodeId _id(id.getValue());
        } catch (Exception &) {
            result = false;
        }

        if (result != id.isValid()) {
            FAIL();
        }
    }
}
