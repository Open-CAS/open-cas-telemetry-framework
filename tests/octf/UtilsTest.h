/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef TESTS_OCTF_UTILSTEST_H
#define TESTS_OCTF_UTILSTEST_H

#include <gtest/gtest.h>
#include <octf/octf.h>

static inline void SetupTestOutput(const testing::TestInfo *info) {
    std::string prefix = "";

    prefix += "[";
    prefix += info->test_case_name();
    prefix += ".";
    prefix += info->name();
    prefix += "]";

    octf::log::cout << octf::log::prefix << prefix;
    octf::log::cerr << octf::log::prefix << prefix;
    octf::log::critical << octf::log::prefix << prefix;
    octf::log::verbose << octf::log::prefix << prefix;
}

#endif  // TESTS_OCTF_UTILSTEST_H
