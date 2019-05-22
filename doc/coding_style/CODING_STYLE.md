# Coding Style of Open CAS Telemetry Framework

## Overview

Help us keep OCTF style unified. We would like to require as simple formatting
as possible. Thus our goal is introducing tools which automate formatting.
We provide git pre-commit hook which you can enable in order to achieve
auto-formatting of your code.

The pre-commit hook performs:
* Auto formatting by invoking clang-format, configuration file for formatting
is located [here](https://github.com/Open-CAS/open-cas-telemetry-framework/blob/master/doc/coding_style/clang-format)
* Correcting include guards
* Adding license header if missing

Bellow there are coding style rules. Each is marked as a rule which can be
automatically ensured - __[A]__, or has to be assured manually - __[M]__.

## Documentation

OCTF requires Doxygen documentation style and the example can be found
[here](https://github.com/Open-CAS/open-cas-telemetry-framework/blob/master/doc/coding_style/DoxygenComments.h)

## Headers

- [A] Include appropriate license header at the beginning of each file

- [A] Include guard with full path (format of it:
FULL\_PROJECT\_PATH\_FILEBASENAME\_H\_)

~~~{.cpp}
#ifndef DOC_CODING_STYLE_CODINGSTYLE_H_
#define DOC_CODING_STYLE_CODINGSTYLE_H_
~~~

- [M] Namespace naming in camelCase convention

~~~{.cpp}
namespace octf {
namespace codingStyle {
~~~

- [M] Class name in PascalCase convention

- [M] Typically every class should have Doxygen documentation

~~~{.cpp}
class PrettyBaseClass {
~~~

- [A] Access modifiers with no indentation

~~~{.cpp}
public:
~~~

- [M] Normal indentation with 4 spaces.

- [M] Typically every public method declaration should have Doxygen
documentation

~~~{.cpp}
    PrettyBaseClass();
~~~

- [M] Use virtual keyword for functions that can be overridden by derived
classes, especially all destructors.

~~~{.cpp}
    virtual ~PrettyBaseClass();
~~~

- [M] Methods name in camelCase convention

~~~{.cpp}
    virtual int myMethod(int name, int someOption) = 0;
};
~~~

- [M] Typically every class should have its own header and source file

- [M] Explicitly mark methods which overrides base.

~~~{.cpp}
class PrettyDerivedClass : public PrettyBaseClass {
public:
    PrettyDerivedClass();
    virtual ~PrettyDerivedClass();

    int myMethod(int name, int someOption) override;
~~~

- [M] Only simple getter and setter functions can be defined in headers.

~~~{.cpp}
    int getParameter() {
        return m_parameter;
    }
~~~

- [M] Use prefix m_ for private members, then use camelCase convention

~~~{.cpp}
private:
    int magicInternalFormula(int number);

    int m_parameter;  // [R] use prefix m_ for private members
};
~~~

- [A] When enclosing namespace include its name in comment

~~~{.cpp}
}  // namespace octf
}  // namespace codingStyle
~~~

- [A] When enclosing include guard include its name in comment

~~~{.cpp}
#endif  // DOC_CODING_STYLE_CODINGSTYLE_H_
~~~

## Source

- [A] Include appropriate license header at the beginning of each file

- Includes
    - [M] Include header specific for this source file. Then put an
    empty line to prevent auto-formatting tool from reordering it.
    - [M] Don't use relative, e.g. "../"
    - [A] Next include all external headers in alphabetical order, then
    all project headers in alphabetical order.

~~~{.cpp}
#include "CodingStyle.h"

#include <iostream>
#include <memory>
#include <octf/SomeHeader.h>
~~~

- [A] Opening brace in the same line.

~~~{.cpp}
using namespace std;

namespace octf
namespace codingStyle {
~~~

- [M] Use constexpr or const instead of C-style macro define-s

~~~{.cpp}
constexpr int VERY_IMPORTANT_PARAM = 22;
~~~

- [M] Dynamic allocations should be done with smart pointers.

~~~{.cpp}
PrettyBaseClass::PrettyBaseClass() {
    // [R] D
    unique_ptr<int> uselessDynamicInt(new int(2));

    shared_ptr<int> uselessSharedInt = make_shared<int>(3);
}
~~~

- [M] All casting operations should be used very carefully. If really needed,
use the most restrictive possible type of C++ casts, never use C-style casting:
(double) intVar
- [A] Keep 80 lines margin

~~~{.cpp}
PrettyBaseClass::~PrettyBaseClass() {
    double number = static_cast<double>(*uselessSharedInt
            + *uselessDynamicInt);
}
~~~

- [A] Constructor and members initializer list as follows

~~~{.cpp}

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
~~~

- [M] Pattern to be used for marking to-do tasks:
[TODO] (Who) Provide explanation about todo.

~~~{.cpp}

int PrettyDerivedClass::magicInternalFormula(int number) {
    // [TODO] (JanKowalski) Provide explanation about computation.
    number = number + 3;

    return number;
}

int PrettyDerivedClass::myMethod(int name, int someOption) {
    return magicInternalFormula(m_parameter);
}

}  // namespace octf
}  // namespace codingStyle
~~~

- [M] For local variables use camelCase convention

~~~{.cpp}
int main() {
    octf::codingStyle::PrettyDerivedClass derivedObject;
    octf::codingStyle::PrettyBaseClass &baseRef = derivedObject;
    cout << "Method result: " << baseRef.myMethod(0, 0) << endl;
}
~~~
