/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Iterators.h>

#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

//
// Table Row Iterator
//

bool RowIterator::operator!=(const RowIterator &other) const {
    return m_link != other.m_link;
}

bool RowIterator::operator==(const RowIterator &other) const {
    return m_link == other.m_link;
}

RowIterator RowIterator::operator--(int) {
    RowIterator cpy = *this;
    m_link--;
    return cpy;
}

RowIterator &RowIterator::operator--() {
    m_link--;
    return *this;
}

RowIterator RowIterator::operator++(int) {
    RowIterator cpy = *this;
    m_link++;
    return cpy;
}

RowIterator &RowIterator::operator++() {
    m_link++;
    return *this;
}

Row *RowIterator::operator->() const {
    return &m_link->second;
}

Row &RowIterator::operator*() const {
    return m_link->second;
}

RowIterator::RowIterator(const LinkType &link)
        : IIterator()
        , m_link(link) {}

RowIterator::RowIterator(const RowIterator &other)
        : IIterator()
        , m_link(other.m_link) {}

RowIterator &RowIterator::operator=(const RowIterator &other) {
    if (this != &other) {
        m_link = other.m_link;
    }
    return *this;
}

//
// Table Row const Iterator
//

bool RowIteratorConst::operator!=(const RowIteratorConst &other) const {
    return m_link != other.m_link;
}

bool RowIteratorConst::operator==(const RowIteratorConst &other) const {
    return m_link == other.m_link;
}

RowIteratorConst RowIteratorConst::operator--(int) {
    RowIteratorConst cpy = *this;
    m_link--;
    return cpy;
}

RowIteratorConst &RowIteratorConst::operator--() {
    m_link--;
    return *this;
}

RowIteratorConst RowIteratorConst::operator++(int) {
    RowIteratorConst cpy = *this;
    m_link++;
    return cpy;
}

RowIteratorConst &RowIteratorConst::operator++() {
    m_link++;
    return *this;
}

const Row *RowIteratorConst::operator->() const {
    return &m_link->second;
}

const Row &RowIteratorConst::operator*() const {
    return m_link->second;
}

RowIteratorConst::RowIteratorConst(const LinkType &link)
        : IIteratorConst()
        , m_link(link) {}

RowIteratorConst::RowIteratorConst(const RowIteratorConst &other)
        : IIteratorConst()
        , m_link(other.m_link) {}

RowIteratorConst &RowIteratorConst::operator=(const RowIteratorConst &other) {
    if (this != &other) {
        m_link = other.m_link;
    }
    return *this;
}

//
// Row Cell Iterator
//

bool CellIterator::operator!=(const CellIterator &other) const {
    return (m_link != other.m_link) || (m_id != other.m_id);
}

CellIterator &CellIterator::operator=(const CellIterator &other) {
    if (this != &other) {
        m_link = other.m_link;
        m_id = other.m_id;
    }

    return *this;
}

bool CellIterator::operator==(const CellIterator &other) const {
    return (m_link == other.m_link) && (m_id == other.m_id);
}

CellIterator CellIterator::operator++(int) {
    CellIterator cpy = *this;
    m_link++;
    return cpy;
}

CellIterator CellIterator::operator--(int) {
    CellIterator cpy = *this;
    m_link--;
    return cpy;
}

Cell &CellIterator::operator*() const {
    Addr addr(m_id, m_link->second.getId());
    return m_map[addr];
}

CellIterator::CellIterator(TableMap &map, LinkType link, size_t id)
        : IIterator()
        , m_map(map)
        , m_link(link)
        , m_id(id) {}

Cell *CellIterator::operator->() const {
    Addr addr(m_id, m_link->second.getId());
    return &m_map[addr];
}

CellIterator &CellIterator::operator++() {
    m_link++;
    return *this;
}

CellIterator &CellIterator::operator--() {
    m_link--;
    return *this;
}

CellIterator::CellIterator(const CellIterator &other)
        : IIterator()
        , m_map(other.m_map)
        , m_link(other.m_link)
        , m_id(other.m_id) {}

//
// Row Cell const Iterator
//

bool CellIteratorConst::operator!=(const CellIteratorConst &other) const {
    return (m_link != other.m_link) || (m_id != other.m_id);
}

CellIteratorConst &CellIteratorConst::operator=(
        const CellIteratorConst &other) {
    if (this != &other) {
        m_link = other.m_link;
        m_id = other.m_id;
    }
    return *this;
}

bool CellIteratorConst::operator==(const CellIteratorConst &other) const {
    return (m_link == other.m_link) && (m_id == other.m_id);
}

CellIteratorConst CellIteratorConst::operator++(int) {
    CellIteratorConst cpy = *this;
    m_link++;
    return cpy;
}

CellIteratorConst CellIteratorConst::operator--(int) {
    CellIteratorConst cpy = *this;
    m_link--;
    return cpy;
}

const Cell &CellIteratorConst::operator*() const {
    Addr addr(m_id, m_link->second.getId());
    return m_map[addr];
}

CellIteratorConst::CellIteratorConst(const TableMap &map,
                                     LinkType link,
                                     size_t id)
        : IIteratorConst()
        , m_map(map)
        , m_link(link)
        , m_id(id) {}

const Cell *CellIteratorConst::operator->() const {
    Addr addr(m_id, m_link->second.getId());
    return &m_map[addr];
}

CellIteratorConst &CellIteratorConst::operator++() {
    m_link++;
    return *this;
}

CellIteratorConst &CellIteratorConst::operator--() {
    m_link--;
    return *this;
}

CellIteratorConst::CellIteratorConst(const CellIteratorConst &other)
        : IIteratorConst()
        , m_map(other.m_map)
        , m_link(other.m_link)
        , m_id(other.m_id) {}

}  // namespace table
}  // namespace octf
