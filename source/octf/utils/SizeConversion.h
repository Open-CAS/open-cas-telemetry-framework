/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_SIZECONVERSION_H
#define SOURCE_OCTF_UTILS_SIZECONVERSION_H

#include <cstddef>
#include <cstdint>

namespace octf {

/**
 * @brief Returns sector size in bytes
 */
inline uint64_t getSectorSize() {
    uint64_t sectorSize = 512;
    return sectorSize;
}

/**
 * @brief Converts given number of bytes into MiB, rounding down
 */
inline std::size_t BytesToMiB(const std::size_t size) {
    return size >> 20ULL;
}

/**
 * @brief Converts given number of bytes into MiB, rounding up
 */
inline std::size_t BytesToMiBCeiling(const std::size_t size) {
    return size / (1 << 20ULL) + ((size % (1 << 20ULL)) != 0);
}

/**
 * @brief Converts given number of MiB into bytes
 */
inline std::size_t MiBToBytes(const std::size_t size) {
    return size << 20ULL;
}

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_SIZECONVERSION_H
