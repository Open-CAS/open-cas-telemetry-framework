/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_PROPERTIES_H
#define SOURCE_OCTF_UTILS_TABLE_PROPERTIES_H

namespace octf {
namespace table {

/**
 * @brief Set of table format properties used to print table
 */
struct Properties {
public:
    Properties();
    virtual ~Properties();
    Properties(const Properties &other);
    Properties &operator=(const Properties &other);

    /**
     * Defines output table format
     */
    enum class TableFormat { csv, text };
    /**
     * Table format
     */
    TableFormat format;

    /**
     * Defines border style
     */
    enum class BorderStyle { empty, solid };
    /**
     * Border Style
     */
    BorderStyle border;

    /**
     * Value indicates if table/row print as a table header
     */
    bool header;

    /**
     * @brief Defines built in table formats
     */
    struct Builtins {
        /**
         * @Gets Gets CSV table style
         */
        static const Properties &csv();

        /**
         * @Gets Gets no border table style
         */
        static const Properties &empt();

        /**
         * @Gets Gets border table style
         */
        static const Properties &solid();
    } static builtin;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_PROPERTIES_H
