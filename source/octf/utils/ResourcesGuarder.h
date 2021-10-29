/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_UTILS_RESOURCESGUARDER_H
#define SOURCE_OCTF_UTILS_RESOURCESGUARDER_H

#include <octf/utils/Exception.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @ingroup Utilities
 *
 * @brief Resources Guarder
 *
 * This is help class which allows to avoid running out of resources. It can be
 * used for example when running jobs in multiple threads which requires
 * noticeable machine's resources (memory, number of opened files).
 *
 * Before executing the job, call ResourcesGuarder::lock method which will block
 * if there is not enough resources and wait until they will be available.
 *
 * When job ends, call ResourcesGuarder::unlock to release resource and allow
 * other waiting jobs to be executed
 *
 */
class ResourcesGuarder : public NonCopyable {
public:
    /**
     * @param utilization The level of resources utilization. Allowed value from
     * 0.0 to 1.0. When value equals to 0.0 it will allow only one job to be
     * executed at time, when equals to 1.0 machine will be utilized full and
     * all jobs run concurrently.
     */
    ResourcesGuarder(double utilization = 0.5);
    virtual ~ResourcesGuarder();

    /**
     * @brief Lock resources.
     *
     * If there is not enough resources and wait until they will be available.
     */
    void lock();

    /**
     * @brief Unlock resource and allow other waiting jobs to be executed
     */
    void unlock();

private:
    bool m_locked;
    double m_utilization;
    class Controller;
    Controller &getController();
};

}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_RESOURCESGUARDER_H
