/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ITERATORS_H
#define SOURCE_OCTF_UTILS_TABLE_ITERATORS_H

#include <map>
#include <octf/utils/container/IIterator.h>
#include <octf/utils/container/IIteratorConst.h>

namespace octf {
namespace table {

class Row;
class Column;
class Cell;
class TableMap;

/**
 * @ingroup Table
 * @brief Row iterator
 */
class RowIterator : public IIterator<Row, RowIterator> {
public:
    RowIterator(const RowIterator &other);
    RowIterator &operator=(const RowIterator &other);
    virtual Row &operator*() const override;
    virtual Row *operator->() const override;
    virtual RowIterator &operator++() override;
    virtual RowIterator operator++(int) override;
    virtual RowIterator &operator--() override;
    virtual RowIterator operator--(int) override;
    virtual bool operator==(const RowIterator &other) const override;
    virtual bool operator!=(const RowIterator &other) const override;

private:
    friend class TableMap;
    typedef std::map<size_t, Row>::iterator LinkType;
    RowIterator(const LinkType &link);
    LinkType m_link;
};

/**
 * @ingroup Table
 * @brief Const row iterator
 */
class RowIteratorConst : public IIteratorConst<Row, RowIteratorConst> {
public:
    RowIteratorConst(const RowIteratorConst &other);
    RowIteratorConst &operator=(const RowIteratorConst &other);
    virtual const Row &operator*() const override;
    virtual const Row *operator->() const override;
    virtual RowIteratorConst &operator++() override;
    virtual RowIteratorConst operator++(int) override;
    virtual RowIteratorConst &operator--() override;
    virtual RowIteratorConst operator--(int) override;
    virtual bool operator==(const RowIteratorConst &other) const override;
    virtual bool operator!=(const RowIteratorConst &other) const override;

private:
    friend class TableMap;
    typedef std::map<size_t, Row>::const_iterator LinkType;
    RowIteratorConst(const LinkType &link);
    LinkType m_link;
};

/**
 * @ingroup Table
 * @brief Cell iterator
 */
class CellIterator : public IIterator<Cell, CellIterator> {
public:
    CellIterator(const CellIterator &other);
    CellIterator &operator=(const CellIterator &other);
    virtual Cell &operator*() const override;
    virtual Cell *operator->() const override;
    virtual CellIterator &operator++() override;
    virtual CellIterator operator++(int) override;
    virtual CellIterator &operator--() override;
    virtual CellIterator operator--(int) override;
    virtual bool operator==(const CellIterator &other) const override;
    virtual bool operator!=(const CellIterator &other) const override;

private:
    friend class TableMap;
    typedef std::map<size_t, Column>::iterator LinkType;
    CellIterator(TableMap &map, LinkType link, size_t id);
    TableMap &m_map;
    LinkType m_link;
    size_t m_id;
};

/**
 * @ingroup Table
 * @brief Const cell iterator
 */
class CellIteratorConst : public IIteratorConst<Cell, CellIteratorConst> {
public:
    CellIteratorConst(const CellIteratorConst &other);
    CellIteratorConst &operator=(const CellIteratorConst &other);
    virtual const Cell &operator*() const override;
    virtual const Cell *operator->() const override;
    virtual CellIteratorConst &operator++() override;
    virtual CellIteratorConst operator++(int) override;
    virtual CellIteratorConst &operator--() override;
    virtual CellIteratorConst operator--(int) override;
    virtual bool operator==(const CellIteratorConst &other) const override;
    virtual bool operator!=(const CellIteratorConst &other) const override;

private:
    friend class TableMap;
    typedef std::map<size_t, Column>::const_iterator LinkType;
    CellIteratorConst(const TableMap &map, LinkType link, size_t id);
    const TableMap &m_map;
    LinkType m_link;
    size_t m_id;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ITERATORS_H
