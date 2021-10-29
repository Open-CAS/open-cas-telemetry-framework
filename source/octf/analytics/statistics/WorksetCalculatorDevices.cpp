/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/analytics/statistics/WorksetCalculatorDevices.h>
#include <octf/utils/Exception.h>

namespace octf {

WorksetCalculatorDevices::WorksetCalculatorDevices()
        : m_devWc() {}

WorksetCalculatorDevices::WorksetCalculatorDevices(
        const WorksetCalculatorDevices &other)
        : m_devWc(other.m_devWc) {}

WorksetCalculatorDevices &octf::WorksetCalculatorDevices::operator=(
        const WorksetCalculatorDevices &other) {
    if (this != &other) {
        m_devWc = other.m_devWc;
    }
    return *this;
}

WorksetCalculatorDevices &octf::WorksetCalculatorDevices::operator=(
        WorksetCalculatorDevices &&other) {
    if (this != &other) {
        m_devWc = std::move(other.m_devWc);
    }
    return *this;
}

WorksetCalculatorDevices::WorksetCalculatorDevices(
        WorksetCalculatorDevices &&other)
        : m_devWc(std::move(other.m_devWc)) {}

void WorksetCalculatorDevices::insertRange(uint64_t devId,
                                           uint64_t begin,
                                           uint64_t length) {
    auto &workset = getWorksetCalculatorByDevice(devId);
    workset.insertRange(begin, length);
}

uint64_t WorksetCalculatorDevices::removeRange(uint64_t devId,
                                               uint64_t begin,
                                               uint64_t length) {
    auto &workset = getWorksetCalculatorByDevice(devId);
    return workset.removeRange(begin, length);
}

uint64_t WorksetCalculatorDevices::getWorkset() const {
    uint64_t sum = 0;

    for (const auto &workset : m_devWc) {
        sum += workset.second.getWorkset();
    }

    return sum;
}

WorksetCalculator &WorksetCalculatorDevices::getWorksetCalculatorByDevice(
        uint64_t devId) {
    auto iter = m_devWc.find(devId);

    if (iter == m_devWc.end()) {
        auto result =
                m_devWc.emplace(std::make_pair(devId, WorksetCalculator()));

        if (!result.second || result.first == m_devWc.end()) {
            throw Exception("Cannot create working set calculator for device");
        }

        return result.first->second;
    }

    return iter->second;
}

}  // namespace octf
