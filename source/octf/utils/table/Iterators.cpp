/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/table/Iterators.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

//
// Table Row Iterator
//

bool RowIterator::operator!=(const RowIterator &other) const {
    return m_iter != other.m_iter;
}

bool RowIterator::operator==(const RowIterator &other) const {
    return m_iter == other.m_iter;
}

RowIterator RowIterator::operator--(int) {
    RowIterator cpy = *this;
    m_iter--;
    return cpy;
}

RowIterator &RowIterator::operator--() {
    m_iter--;
    return *this;
}

RowIterator RowIterator::operator++(int) {
    RowIterator cpy = *this;
    m_iter++;
    return cpy;
}

RowIterator &RowIterator::operator++() {
    m_iter++;
    return *this;
}

Row *RowIterator::operator->() const {
    return &m_iter->second;
}

Row &RowIterator::operator*() const {
    return m_iter->second;
}

RowIterator::RowIterator(const iter_t &link)
        : IIterator()
        , m_iter(link) {}

RowIterator::RowIterator(const RowIterator &other)
        : IIterator()
        , m_iter(other.m_iter) {}

RowIterator &RowIterator::operator=(const RowIterator &other) {
    if (this != &other) {
        m_iter = other.m_iter;
    }
    return *this;
}

//
// Table Row const Iterator
//

bool RowIteratorConst::operator!=(const RowIteratorConst &other) const {
    return m_iter != other.m_iter;
}

bool RowIteratorConst::operator==(const RowIteratorConst &other) const {
    return m_iter == other.m_iter;
}

RowIteratorConst RowIteratorConst::operator--(int) {
    RowIteratorConst cpy = *this;
    m_iter--;
    return cpy;
}

RowIteratorConst &RowIteratorConst::operator--() {
    m_iter--;
    return *this;
}

RowIteratorConst RowIteratorConst::operator++(int) {
    RowIteratorConst cpy = *this;
    m_iter++;
    return cpy;
}

RowIteratorConst &RowIteratorConst::operator++() {
    m_iter++;
    return *this;
}

const Row *RowIteratorConst::operator->() const {
    return &m_iter->second;
}

const Row &RowIteratorConst::operator*() const {
    return m_iter->second;
}

RowIteratorConst::RowIteratorConst(const iter_t &link)
        : IIteratorConst()
        , m_iter(link) {}

RowIteratorConst::RowIteratorConst(const RowIteratorConst &other)
        : IIteratorConst()
        , m_iter(other.m_iter) {}

RowIteratorConst &RowIteratorConst::operator=(const RowIteratorConst &other) {
    if (this != &other) {
        m_iter = other.m_iter;
    }
    return *this;
}

//
// Row Cell Iterator
//

bool CellIterator::operator!=(const CellIterator &other) const {
    return (m_iter != other.m_iter) || (m_rowIndex != other.m_rowIndex);
}

CellIterator &CellIterator::operator=(const CellIterator &other) {
    if (this != &other) {
        m_iter = other.m_iter;
        m_rowIndex = other.m_rowIndex;
    }

    return *this;
}

bool CellIterator::operator==(const CellIterator &other) const {
    return (m_iter == other.m_iter) && (m_rowIndex == other.m_rowIndex);
}

CellIterator CellIterator::operator++(int) {
    CellIterator cpy = *this;
    m_iter++;
    return cpy;
}

CellIterator CellIterator::operator--(int) {
    CellIterator cpy = *this;
    m_iter--;
    return cpy;
}

Cell &CellIterator::operator*() const {
    Addr addr(m_rowIndex, m_iter->second.getIndex());
    return m_map[addr];
}

CellIterator::CellIterator(TableMap &map, iter_t link, index_t rowIndex)
        : IIterator()
        , m_map(map)
        , m_iter(link)
        , m_rowIndex(rowIndex) {}

Cell *CellIterator::operator->() const {
    Addr addr(m_rowIndex, m_iter->second.getIndex());
    return &m_map[addr];
}

CellIterator &CellIterator::operator++() {
    m_iter++;
    return *this;
}

CellIterator &CellIterator::operator--() {
    m_iter--;
    return *this;
}

CellIterator::CellIterator(const CellIterator &other)
        : IIterator()
        , m_map(other.m_map)
        , m_iter(other.m_iter)
        , m_rowIndex(other.m_rowIndex) {}

//
// Row Cell const Iterator
//

bool CellIteratorConst::operator!=(const CellIteratorConst &other) const {
    return (m_iter != other.m_iter) || (m_rowIndex != other.m_rowIndex);
}

CellIteratorConst &CellIteratorConst::operator=(
        const CellIteratorConst &other) {
    if (this != &other) {
        m_iter = other.m_iter;
        m_rowIndex = other.m_rowIndex;
    }
    return *this;
}

bool CellIteratorConst::operator==(const CellIteratorConst &other) const {
    return (m_iter == other.m_iter) && (m_rowIndex == other.m_rowIndex);
}

CellIteratorConst CellIteratorConst::operator++(int) {
    CellIteratorConst cpy = *this;
    m_iter++;
    return cpy;
}

CellIteratorConst CellIteratorConst::operator--(int) {
    CellIteratorConst cpy = *this;
    m_iter--;
    return cpy;
}

const Cell &CellIteratorConst::operator*() const {
    Addr addr(m_rowIndex, m_iter->second.getIndex());
    return m_map[addr];
}

CellIteratorConst::CellIteratorConst(const TableMap &map,
                                     iter_t link,
                                     index_t rowIndex)
        : IIteratorConst()
        , m_map(map)
        , m_iter(link)
        , m_rowIndex(rowIndex) {}

const Cell *CellIteratorConst::operator->() const {
    Addr addr(m_rowIndex, m_iter->second.getIndex());
    return &m_map[addr];
}

CellIteratorConst &CellIteratorConst::operator++() {
    m_iter++;
    return *this;
}

CellIteratorConst &CellIteratorConst::operator--() {
    m_iter--;
    return *this;
}

CellIteratorConst::CellIteratorConst(const CellIteratorConst &other)
        : IIteratorConst()
        , m_map(other.m_map)
        , m_iter(other.m_iter)
        , m_rowIndex(other.m_rowIndex) {}

}  // namespace table
}  // namespace octf
