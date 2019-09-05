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
    WorksetCalculator &operator=(WorksetCalculator &&other);

    /**
     * @brief Inserts a range to be calculated into workset
     *
     * @note Range with a length of zero is ignored
     *
     * @param begin Starting range value
     * @param length Length of range
     */
    void insertRange(uint64_t begin, uint64_t length);

    /**
     * @brief Removes range from the calculator.
     *
     * @note The workset is not decreased, because  we keep a maximum value
     * achieved, which is returned if it is bigger than kept workset.
     *
     * @param begin Starting range value
     * @param length Length of range
     * @return Total length of ranges removed
     */
    uint64_t removeRange(uint64_t begin, uint64_t length);

    /**
     * @return Maximum achieved workset of all given ranges
     *
     * Because we allow for removing ranges we also keep a max value of achieved
     * max workset. This logic is valid from caching perspective. Consider the
     * following example:
     *  - We write 100 sector range starting at address 0
     *  - We remove this range
     *  - We write another 100 sector range at address 100
     *
     *  Considering different approaches, the workset could be 100 or 200
     *  sectors. This calculator chooses the former, as considering caching
     *  mechanisms, we could put the second range in place of the first range.
     */
    uint64_t getWorkset() const;

private:
    struct Range {
        uint64_t begin;
        uint64_t end;

        Range();
        Range(uint64_t _begin, uint64_t _end);
        Range(const Range &other);
        Range(Range &&other);
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

    /**
     * Max acheived workset
     */
    uint64_t m_max;

    /**
     * Did we add any new ranges since the last workset calculation
     */
    bool m_isMaxFresh;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATOR_H
