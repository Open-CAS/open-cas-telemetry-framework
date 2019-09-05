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

WorksetCalculator::Range::Range(Range &&other)
        : begin(std::move(other.begin))
        , end(std::move(other.end)) {}

WorksetCalculator::Range &WorksetCalculator::Range::operator=(
        const Range &other) {
    if (this != &other) {
        this->begin = other.begin;
        this->end = other.end;
    }
    return *this;
}

WorksetCalculator::Range &WorksetCalculator::Range::operator=(Range &&other) {
    if (this != &other) {
        this->begin = std::move(other.begin);
        this->end = std::move(other.end);
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
 *
 * @note Continuous ranges e.g. ranges [0;1] and [1;2],
 *  are also treated as overlapping
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
        : m_hitRanges()
        , m_max(0)
        , m_isMaxFresh(true) {}

WorksetCalculator::WorksetCalculator(const WorksetCalculator &other)
        : m_hitRanges(other.m_hitRanges)
        , m_max(other.m_max) {}

WorksetCalculator &octf::WorksetCalculator::operator=(
        const WorksetCalculator &other) {
    if (this != &other) {
        m_hitRanges = other.m_hitRanges;
        m_max = other.m_max;
    }
    return *this;
}

WorksetCalculator &octf::WorksetCalculator::operator=(
        WorksetCalculator &&other) {
    if (this != &other) {
        m_hitRanges = std::move(other.m_hitRanges);
        m_max = std::move(other.m_max);
    }
    return *this;
}

WorksetCalculator::WorksetCalculator(WorksetCalculator &&other)
        : m_hitRanges(std::move(other.m_hitRanges))
        , m_max(std::move(other.m_max)) {}

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

    // Maximal achieved value may have changed
    m_isMaxFresh = false;
}

uint64_t WorksetCalculator::getWorkset() const {
    if (m_isMaxFresh == true) {
        return m_max;

    } else {
        uint64_t workset = 0;

        for (const auto &range : m_hitRanges) {
            workset += (range.end - range.begin);
        }

        return std::max(m_max, workset);
    }
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

    // Remember the first range which does not overlap with newRange
    // - we will insert the merged range right before it
    std::set<Range>::const_iterator insertHint = overlapIter;

    // Insert a new range merged from deleted ranges
    Range merged(minBegin, maxEnd);
    m_hitRanges.insert(insertHint, std::move(merged));
}

uint64_t WorksetCalculator::removeRange(uint64_t begin, uint64_t length) {
    // Create range to be removed
    Range rangeToRemove(begin, begin + length);
    Range splitBegin, splitEnd;
    bool removeRange = false;
    uint64_t lengthRemoved = 0;

    auto iter = m_hitRanges.find(rangeToRemove);
    if (iter == m_hitRanges.end()) {
        return 0;
    }

    // Remember the max achieved value of workset before removing
    if (m_isMaxFresh == false) {
        m_max = std::max(getWorkset(), m_max);
        m_isMaxFresh = true;
    }

    while (iter != m_hitRanges.end() &&
           Range::doRangesOverlap(rangeToRemove, *iter)) {
        // Don't remove ranges which are just continuous - e.g. [0;1] and [1;2]
        removeRange = false;

        if (iter->begin < rangeToRemove.begin) {
            removeRange = true;
            splitBegin.begin = iter->begin;
            splitBegin.end = rangeToRemove.begin;
        }

        if (iter->end > rangeToRemove.end) {
            removeRange = true;
            splitEnd.begin = rangeToRemove.end;
            splitEnd.end = iter->end;

        } else if (iter->end == rangeToRemove.end &&
                   iter->begin == rangeToRemove.begin) {
            removeRange = true;
        }

        if (removeRange) {
            lengthRemoved += iter->end - iter->begin;

            // Remove overlapping range
            iter = m_hitRanges.erase(iter);

            // Add any split ranges which were a part of the removed range
            if (splitBegin.begin != splitBegin.end) {
                lengthRemoved -= splitBegin.end - splitBegin.begin;
                m_hitRanges.insert(splitBegin);
            }

            if (splitEnd.begin != splitEnd.end) {
                lengthRemoved -= splitEnd.end - splitEnd.begin;
                m_hitRanges.insert(splitEnd);
            }

        } else {
            iter++;
        }
    }

    return lengthRemoved;
}

}  // namespace octf
