/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_TYPES_H
#define SOURCE_OCTF_UTILS_TABLE_TYPES_H

#include <stddef.h>

/*
 * Forward declaration for Row's stream operators
 */
namespace google {
namespace protobuf {
class Descriptor;
class Message;
class Reflection;
}  // namespace protobuf
}  // namespace google

namespace octf {
namespace table {

class Row;
class Column;
class Cell;
class TableMap;

/**
 * @ingroup Table
 * @brief Index type for indexing rows and columns in Table
 */
typedef size_t index_t;

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_TYPES_H
