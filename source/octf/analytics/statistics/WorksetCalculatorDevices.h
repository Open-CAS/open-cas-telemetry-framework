/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATORDEVICES_H
#define SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATORDEVICES_H

#include <map>
#include <octf/analytics/statistics/WorksetCalculator.h>

namespace octf {

/**
 * @brief Utility class to calculate working set from inserted ranges for
 * multiple devices
 *
 * Workset is defined here as a length of union of all inputed ranges.
 * Ranges are defined using beginning number and length similarly to how I/O
 * requests are defined.
 */
class WorksetCalculatorDevices {
public:
    WorksetCalculatorDevices();
    virtual ~WorksetCalculatorDevices() = default;
    WorksetCalculatorDevices(const WorksetCalculatorDevices &);
    WorksetCalculatorDevices(WorksetCalculatorDevices &&other);
    WorksetCalculatorDevices &operator=(const WorksetCalculatorDevices &other);
    WorksetCalculatorDevices &operator=(WorksetCalculatorDevices &&other);

    /**
     * @brief Inserts a range to be calculated into working set
     *
     * @note Range with a length of zero is ignored
     *
     * @param devId Device id for which rage is inserted
     * @param begin Starting range value
     * @param length Length of range
     */
    void insertRange(uint64_t devId, uint64_t begin, uint64_t length);

    /**
     * @brief Removes range from the calculator.
     *
     * @note The working set is not decreased, because  we keep a maximum value
     * achieved, which is returned if it is bigger than kept working set.
     *
     * @param devId Device id for which rage is removed
     * @param begin Starting range value
     * @param length Length of range
     *
     * @return Total length of ranges removed
     */
    uint64_t removeRange(uint64_t devId, uint64_t begin, uint64_t length);

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
    WorksetCalculator &getWorksetCalculatorByDevice(uint64_t devId);

private:
    /**
     * Map of working set calculator by device id
     */
    std::map<uint64_t, WorksetCalculator> m_devWc;
};

}  // namespace octf

#endif  // SOURCE_OCTF_ANALYTICS_STATISTICS_WORKSETCALCULATORDEVICES_H
