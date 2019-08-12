/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/analytics/statistics/WorksetCalculator.h>

namespace octf {

WorksetCalculator::Range::Range()
        : begin(0)
        , end(0) {}

WorksetCalculator::Range::Range(uint64_t begin, uint64_t end)
        : begin(begin)
        , end(end) {}

WorksetCalculator::Range::Range(const Range &other)
        : begin(other.begin)
        , end(other.end) {}

WorksetCalculator::Range &WorksetCalculator::Range::operator=(
        const Range &other) {
    if (this != &other) {
        this->begin = other.begin;
        this->end = other.end;
    }
    return *this;
}

/**
 * @brief Ranges are equal if they overlap
 */
bool WorksetCalculator::Range::operator==(const Range &other) const {
    if (doRangesOverlap(*this, other)) {
        return true;
    }
    return false;
}

/**
 * @brief Does range r1 overlap with r2
 */
bool WorksetCalculator::Range::doRangesOverlap(const Range &r1,
                                               const Range &r2) {
    if (r1.begin <= r2.end && r1.end >= r2.begin) {
        return true;
    }
    return false;
}

/**
 * @brief Allow sorting in ascending order by begin, overlapping ranges are
 * considered equal
 */
bool WorksetCalculator::Range::operator<(const Range &right) const {
    if (doRangesOverlap(*this, right)) {
        return false;
    }

    return this->begin < right.begin;
}

WorksetCalculator::WorksetCalculator()
        : m_hitRanges() {}

WorksetCalculator::WorksetCalculator(const WorksetCalculator &)
        : m_hitRanges() {}

WorksetCalculator &octf::WorksetCalculator::operator=(
        const WorksetCalculator &other) {
    m_hitRanges = other.m_hitRanges;
    return *this;
}
void WorksetCalculator::insertRange(uint64_t begin, uint64_t len) {
    // Ignore ranges with 0 length
    if (len == 0) {
        return;
    }

    // Look for ranges which overlap
    Range newRange(begin, begin + len);
    auto iter = m_hitRanges.find(newRange);

    if (iter != m_hitRanges.end()) {
        // Remove overlapping ranges and insert a merged one
        mergeRanges(newRange, iter);

    } else {
        // No overlapping ranges, just insert new range
        m_hitRanges.insert(newRange);
    }
}

uint64_t WorksetCalculator::getWorkset() const {
    uint64_t workset = 0;

    for (const auto &range : m_hitRanges) {
        workset += (range.end - range.begin);
    }

    return workset;
}

void WorksetCalculator::mergeRanges(const Range &newRange,
                                    std::set<Range>::iterator &overlapIter) {
    uint64_t minBegin = std::min(overlapIter->begin, newRange.begin);
    uint64_t maxEnd = std::max(overlapIter->end, newRange.end);

    // If new range is subrange of existing range, return
    if (newRange.begin >= overlapIter->begin &&
        newRange.end <= overlapIter->end) {
        return;
    }

    // Delete the overlapping ranges
    overlapIter = m_hitRanges.erase(overlapIter);

    while (overlapIter != m_hitRanges.end() &&
           Range::doRangesOverlap(newRange, *overlapIter)) {
        maxEnd = std::max(maxEnd, overlapIter->end);
        overlapIter = m_hitRanges.erase(overlapIter);
    }

    // Remember the first range which does not overlap wih newRange
    // - we will insert the merged range right before it
    std::set<Range>::const_iterator insertHint = overlapIter;

    // Insert a new range merged from deleted ranges
    Range merged(minBegin, maxEnd);
    m_hitRanges.insert(insertHint, std::move(merged));
}

}  // namespace octf
