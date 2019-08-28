/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATOR_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATOR_H
#include <cstdint>
#include <set>

namespace octf {

/**
 * @brief Utility class to calculate workset from inserted ranges
 *
 * Workset is defined here as a length of union of all inputed ranges.
 * Ranges are defined using beginning number and length similarily to how I/O
 * requests are defined.
 */
class WorksetCalculator {
public:
    WorksetCalculator();
    virtual ~WorksetCalculator() = default;
    WorksetCalculator(const WorksetCalculator &);
    WorksetCalculator(WorksetCalculator &&other);
    WorksetCalculator &operator=(const WorksetCalculator &other);

    /**
     * @brief Insert a range to be calculated into workset
     *
     * @note Range with a length of zero is ignored
     *
     * @param begin Starting range number
     * @param length Length of range
     */
    void insertRange(uint64_t begin, uint64_t length);

    /**
     * @return Workset of all given ranges
     */
    uint64_t getWorkset() const;

private:
    struct Range {
        uint64_t begin;
        uint64_t end;

        Range();
        Range(uint64_t _begin, uint64_t _end);
        Range(const Range &other);
        Range &operator=(const Range &other);
        Range &operator=(Range &&other);
        bool operator==(const Range &other) const;
        bool operator<(const Range &right) const;
        static bool doRangesOverlap(const Range &r1, const Range &r2);
    };

    void mergeRanges(const Range &newRange,
                     std::set<Range>::iterator &overlapIter);

    /**
     * Set of ranges
     */
    std::set<Range> m_hitRanges;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATOR_H
