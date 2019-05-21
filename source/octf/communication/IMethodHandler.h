/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_IMETHODHANDLER_H
#define SOURCE_OCTF_COMMUNICATION_IMETHODHANDLER_H
#include <memory>

namespace octf {

class Method;

/**
 * @typedef Shared pointer of method to be executed by service/plugin
 */
typedef std::shared_ptr<Method> MethodShRef;

/**
 * @interface IMethodHandler
 *
 * Using this interface it will be possible to receive methods sent from client
 * to service/plugin and execute them
 */
class IMethodHandler {
public:
    IMethodHandler() = default;
    virtual ~IMethodHandler() = default;

    /**
     * @brief Handles (execute) method sent from client to service/plugin
     *
     * @param method Method to be handled
     */
    virtual void handleMethod(const MethodShRef &method) = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_IMETHODHANDLER_H
