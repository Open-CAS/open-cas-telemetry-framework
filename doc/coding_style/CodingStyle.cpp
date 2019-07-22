/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * [A] Include appropriate license header at the beginning of each file
 */

// [M] means a coding-style rule which has to be assured manually
// [A] means a rule which can be automatically ensured

// Includes:
//    - [M] Include header specific for this source file. Then put an
//          empty line to prevent auto-formatting tool from reordering it.
//    - [M] Don't use relative shortcuts, e.g. "../"
//    - [A] Next include all external headers in alphabetical order, then
//          all project headers in alphabetical order.

#include "CodingStyle.h"

#include <iostream>
#include <memory>
#include <octf/SomeHeader.h>
using namespace std;

namespace octf {  // [A] Opening brace in the same line.
namespace codingStyle {

// [M] Use constexpr or const instead of C-style macro define-s
constexpr int VERY_IMPORTANT_PARAM = 22;

PrettyBaseClass::PrettyBaseClass() {
    // [M] Dynamic allocations should be done
    // with smart pointers.
    unique_ptr<int> uselessDynamicInt(new int(2));

    shared_ptr<int> uselessSharedInt = make_shared<int>(3);
}

PrettyBaseClass::~PrettyBaseClass() {
    // [M] All casting operations should be used very carefully.
    // If really needed, use the most restrictive possible type
    // of C++ casts, never use C-style casting: (double) intVar

    // [A] Keep 80 lines margin

    double number =
            static_cast<double>(*uselessSharedInt + *uselessDynamicInt) +
            yetAnotherInt;
}

// [M] Typically every class should has its own header and source file.
// For convenience of readers, let's ignore this rule here.

// [A] Constructor and members initializer list as follows
PrettyDerivedClass::PrettyDerivedClass()
        : PrettyBaseClass()
        , m_parameter(2) {
    if (true) {
        m_parameter++;
    } else {
        m_parameter--;
    }
}

PrettyDerivedClass::~PrettyDerivedClass() {}

int PrettyDerivedClass::magicInternalFormula(int number) {
    number = number + 3;

    // [M] Pattern to be used for marking to-do tasks:
    // TODO (jankowalski) Provide explanation about computation.

    return number;
}

int PrettyDerivedClass::myMethod(int name, int someOption) {
    return magicInternalFormula(m_parameter);
}

// [A] Closing brace of namespace should be commented with its name
}  // namespace codingStyle
}  // namespace octf

int main() {
    octf::codingStyle::PrettyDerivedClass derivedObject;
    octf::codingStyle::PrettyBaseClass &baseRef = derivedObject;
    cout << "Method result: " << baseRef.myMethod(0, 0) << endl;
}
