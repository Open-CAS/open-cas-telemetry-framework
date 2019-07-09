/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_ITERATORS_H
#define SOURCE_OCTF_UTILS_TABLE_ITERATORS_H

#include <map>
#include <octf/utils/container/IIterator.h>
#include <octf/utils/container/IIteratorConst.h>
#include <octf/utils/table/Types.h>

namespace octf {
namespace table {

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
    typedef std::map<index_t, Row>::iterator iter_t;
    RowIterator(const iter_t &link);
    iter_t m_iter;
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
    typedef std::map<index_t, Row>::const_iterator iter_t;
    RowIteratorConst(const iter_t &link);
    iter_t m_iter;
};

/**
 * @ingroup Table
 * @brief Cell iterator of row
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
    typedef std::map<index_t, Column>::iterator iter_t;
    CellIterator(TableMap &map, iter_t link, index_t rowIndex);
    TableMap &m_map;
    iter_t m_iter;
    index_t m_rowIndex;
};

/**
 * @ingroup Table
 * @brief Const cell iterator of row
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
    typedef std::map<index_t, Column>::const_iterator iter_t;
    CellIteratorConst(const TableMap &map, iter_t link, index_t rowIndex);
    const TableMap &m_map;
    iter_t m_iter;
    index_t m_rowIndex;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_ITERATORS_H
