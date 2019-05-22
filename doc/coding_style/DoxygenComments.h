/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef DOC_CODING_STYLE_DOXYGENCOMMENTS_H
#define DOC_CODING_STYLE_DOXYGENCOMMENTS_H

/**
 * (Optional) @file
 *
 * File documentation
 */

namespace octf {
namespace codingStyle {

/**
 * @brief Short description of class
 *
 * Detailed description of class and it's nomenclature
 */
class SomeClass {
public:
    /**
     * @brief Constructor
     *
     * Detailed description if appropriate
     */
    SomeClass();

    /**
     * @brief Constructor creating someClass with someParameter
     *
     * Detailed description if appropriate
     *
     * @param someParameter Parameter meaning
     */
    SomeClass(int someParameter);

    /**
     * @brief Destructor
     *
     * Detailed description if appropriate
     */
    virtual ~SomeClass();

    /**
     * @brief What this method does
     * (note: 3rd person usage, e.g. 'Adds values' not 'Add values')
     *
     * Detailed description if appropriate
     *
     * @return General return value meaning
     * @retval <0 Specific value meaning
     * @retval 0 Specific value meaning
     * @retval 1 Specific value meaning
     * @retval >1 Specific value meaning
     */
    int methodReturningImportantCode();

    /**
     * @brief What this method does
     * (note: 3rd person usage, e.g. 'Adds values' not 'Add values')
     *
     * Detailed description if appropriate
     *
     * @param inParameter Meaning (by default parameters with no tag are input
     * parameters)
     * @param[out] outputParameter Meaning
     */
    void methodWithOutputParameter(int inParameter, int &outputParameter);

    /**
     * @return What is the meaning of this return
     */
    int getSomeMember();

    /**
     * @param something What this parameter means
     */
    void setSomeMember(bool something);

private:
    /**
     * (Optional) @brief What this method does
     * (note: 3rd person usage, e.g. 'Adds values' not 'Add values')
     *
     * (Optional) Detailed description if appropriate
     *
     * (Optional) @return Return value meaning
     */
    int privateMethod();

private:
    /**
     * (Optional) Description of this member's usage
     */
    int m_someMember;
};

}  // namespace codingStyle
}  // namespace octf

#endif  // DOC_CODING_STYLE_DOXYGENCOMMENTS_H
