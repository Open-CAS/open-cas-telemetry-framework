/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_TABLE_H
#define SOURCE_OCTF_UTILS_TABLE_TABLE_H

#include <map>
#include <string>
#include <octf/utils/Log.h>
#include <octf/utils/NonCopyable.h>
#include <octf/utils/container/IContainer.h>
#include <octf/utils/table/Cell.h>
#include <octf/utils/table/Column.h>
#include <octf/utils/table/Iterators.h>
#include <octf/utils/table/Row.h>
#include <octf/utils/table/Types.h>

/*
 * Forward declaration for Row's stream operators
 */
namespace google {
namespace protobuf {
class Descriptor;
class Message;
}  // namespace protobuf
}  // namespace google

namespace octf {
namespace table {

/**
 * @defgroup Table Table
 * A very flexible table assuring dynamic number or rows and columns
 *
 * @ingroup Utilities
 */

/**
 * @ingroup Table
 * @brief A very flexible table
 *
 * The table is capable to keep dynamically rows and columns. It provides double
 * array operator [][] for accessing cells. Cells may stores any type of value.
 *
 * @code
 * // Define the table
 * octf::table::Table table;
 *
 * // Fill table
 * table[0][0] = "A string";
 * table[1][1] = -1;
 * table[2][2] = 3.14;
 * table[0][0] = table[2][2];
 *
 * // Use indexing table by strings
 * table["Row Name"]["Column Name"] = 2.71;
 *
 * // Print table
 * octf::log << table << std::endl;
 * @endcode
 */
class Table : public NonCopyable,
              public IContainer<Row, RowIterator, RowIteratorConst> {
public:
    Table();
    virtual ~Table();

    /**
     * @brief Gets row by row index
     * @param row Row index
     * @return Row
     */
    Row &operator[](index_t row);

    /**
     * @brief Gets row by row name
     * @param row Row name
     * @return Row
     */
    Row &operator[](const std::string &row);

    /**
     * @brief Gets row by row index
     * @param row Row index
     * @return Row
     */
    const Row &operator[](index_t row) const;

    /**
     * @brief Gets row by row name
     * @param row Row name
     * @return Row
     */
    const Row &operator[](const std::string &row) const;

    void clear() override;

    index_t size() const override;

    bool empty() const override;

    RowIterator begin() override;

    RowIterator end() override;

    RowIteratorConst begin() const override;

    RowIteratorConst end() const override;

private:
    std::unique_ptr<TableMap> m_map;
};

/**
 * @ingroup Table
 *
 * @brief Output stream operator of Table
 *
 * @param os output stream
 * @param table Table to be streamed
 *
 * @return Reference to stream
 */
std::ostream &operator<<(std::ostream &os, const Table &table);

/**
 * @ingroup Table
 *
 * @brief Streams protocol buffer message description to the row
 *
 * For the following message:
 * @code
 * message Bar {
 *     uint32 type = 1;
 *
 *     string name = 2;
 * }
 *
 * message Foo {
 *     uint64 timestamp = 1;
 *
 *     Bar bar = 2;
 * }
 * @endcode
 *
 * executing the code:
 * @code
 * // Define the table
 * octf::table::Table table;
 *
 * // Define message
 * Foo foo;
 *
 * // Fill table's header (row 0)
 * table[0] << foo.GetDescriptor();
 * @endcode
 *
 * it is equivalent to the following sequence:
 * @code
 * row["timestamp"] = "timestamp";
 * row["bar.type"] = "bar.type";
 * row["name"] = "bar.name";
 * @endcode
 *
 * Printing table to the output stream:
 * @code
 * octf::log << table << std::endl;
 * @endcode
 * will result in:
 * @code
 * timestamp,bar.type,bar.name
 * @endcode
 *
 * @param row row into which stream protocol buffer message description
 * @param desc message description to be streamed
 *
 * @note At the moment map and repeated fields in message are not supported,
 * skipped, and not printed.
 *
 * @return Reference to the row
 */
Row &operator<<(Row &row, const ::google::protobuf::Descriptor *desc);

/**
 * @ingroup Table
 *
 * @brief Streams protocol buffer message to the row
 *
 * For the following message:
 * @code
 * message Bar {
 *     uint32 type = 1;
 *
 *     string name = 2;
 * }
 *
 * message Foo {
 *     uint64 timestamp = 1;
 *
 *     Bar bar = 2;
 * }
 * @endcode
 *
 * executing the code:
 * @code
 * // Define the table
 * octf::table::Table table;
 *
 * // Define messages and set them
 * Foo foo;
 * foo.set_timestamp(100);
 * foo.mutable_bar()->set_type(7);
 * foo.mutable_bar()->set_name("Galaxy");
 *
 * Bar bar;
 * bar.set_type(33);
 * bar.set_name("Star");
 *
 * // Fill table's header (row 0)
 * table[0] << foo.GetDescriptor() << bar.GetDescriptor();
 *
 * // Fill table's rows
 * table[1] << foo;
 * table[2] << bar;
 * table[3] << bar << foo;
 *
 * octf::log << table << std::endl;
 * @endcode
 *
 * will print following:
 * @code
 * timestamp,bar.type,bar.name,type,bar
 * 100,7,Galaxy,,
 * ,,,33,Star
 * 100,7,Galaxy,33,Star
 * @endcode
 *
 * @param row row into which stream protocol buffer message
 * @param desc message to be streamed
 *
 * @note At the moment map and repeated fields in message are not supported,
 * skipped, and not printed
 *
 * @return Reference to the row
 */
Row &operator<<(Row &row, const ::google::protobuf::Message &msg);

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_TABLE_H
