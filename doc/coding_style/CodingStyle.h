/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * [A] Include appropriate license header at the beginning of each file
 */

#ifndef DOC_CODING_STYLE_CODINGSTYLE_H
#define DOC_CODING_STYLE_CODINGSTYLE_H
// [M] means a coding-style rule which has to be assured manually
// [A] means a rule which can be automatically ensured

// [A] Include guard in form FULL_PROJECT_PATH_FILEBASENAME_H_

#include <SomeRequiredSystemHeader>
#include "SomeRequiredLocalHeader.h"

namespace octf {  // [M] Namespace naming in camel case convention
namespace codingStyle {

// [M] Typically every class should have doxygen documentation in form of
// comment. For convenience of readers, guidelines for these comments are in
// DoxygenComments.h
class PrettyBaseClass {
public:  // [A] Access modifiers with no indentation.
    // [M] Typically every public method declaration should have doxygen
    // documentation in form of comment. For convenience of readers,
    // guidelines for these comments are in doxygenComments.h

    // [A] Normal indentation with 4 spaces.
    PrettyBaseClass();

    // [M] Use virtual keyword for functions that can be overridden
    // by derived classes, especially all destructors.
    virtual ~PrettyBaseClass();

    // [M] Methods name in camelCase convention.
    virtual int myMethod(int name, int someOption) = 0;
};

// [M] Typically every class should have its own header and source file.
// For convenience of readers, let's ignore this rule here.

class PrettyDerivedClass : public PrettyBaseClass {
public:
    PrettyDerivedClass();
    virtual ~PrettyDerivedClass();

    // [M] Explicitly mark methods which overrides base.
    int myMethod(int name, int someOption) override;

    // [M] Only simple getter and setter functions can be defined in headers.
    int getParameter() {
        return m_parameter;
    }

private:
    int magicInternalFormula(int number);

    int m_parameter;  // Use prefix m_ for private members, then use camelCase
                      // convention
};

// [A] When enclosing namesapce include it name in comment
}  // namespace codingStyle
}  // namespace octf

// [A] When enclosing include guard include it name in comment

#endif  // DOC_CODING_STYLE_CODINGSTYLE_H
