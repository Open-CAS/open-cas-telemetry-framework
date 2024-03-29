/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
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
#include <octf/utils/table/Properties.h>
#include <octf/utils/table/Row.h>
#include <octf/utils/table/Types.h>

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
 * The table is capable of keeping dynamically rows and columns. It provides
 * double array operator [][] for accessing cells. Cells may store any type of
 * value.
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
 * octf::log::cout << table << std::endl;
 * @endcode
 */
class Table : public IContainer<Row, RowIterator, RowIteratorConst> {
public:
    Table();
    virtual ~Table();
    Table(Table const &table);
    Table &operator=(Table const &table);

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

    /**
     * @brief Gets table title
     * @return Table title
     */
    const std::string &getTitle() const {
        return m_title;
    }

    /**
     * @brief Set table title
     * @param title Title of the table to be set
     */
    void setTitle(const std::string &title) {
        m_title = title;
    }

    /**
     * @brief Gets table format properties in read only access
     * @return Table format properties
     */
    const Properties &getProperties() const;

    /**
     * @brief Gets table format properties and modify them
     * @return Table format properties
     */
    Properties &getProperties();

    /**
     * @brief Sets table format properties
     * @param props Table format properties to be set
     */
    void setProperties(const Properties &props);

private:
    std::unique_ptr<TableMap> m_map;
    std::unique_ptr<Properties> m_props;
    std::string m_title;
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
 * table::setHeader(table[0], foo);
 * table::setHeader(table[0], bar);
 *
 * // Fill table's rows
 * table[1] << foo;
 * table[2] << bar;
 * table[3] << bar << foo;
 *
 * octf::log::cout << table << std::endl;
 * @endcode
 *
 * will print following:
 * @code
 * foo.timestamp,foo.type,foo.name,bar.type,bar.name
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

/**
 * @ingroup Table
 * @brief Streams protocol buffer message to the row
 */
inline Row &operator<<(Row &row, const ::google::protobuf::Message *msg) {
    row << *msg;
    return row;
}

/**
 * @ingroup Table
 * @brief Sets header on the basis of protocol buffer message
 *
 * The specified message will be analyzed in terms of content and its
 * description. Then row will be filled with columns' name association
 * respectively.
 *
 * @note When streaming messages with variable size of repeated fields, the
 * header should be set using each message.
 *
 * @param row row into which fill header
 * @param msg protocol buffer message on the basis which header is made
 */
void setHeader(Row &row, const ::google::protobuf::Message *msg);

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_TABLE_H
