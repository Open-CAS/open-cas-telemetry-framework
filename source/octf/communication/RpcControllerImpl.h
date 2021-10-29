/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_RPCCONTROLLERIMPL_H
#define SOURCE_OCTF_COMMUNICATION_RPCCONTROLLERIMPL_H

#include <google/protobuf/service.h>
#include <memory>
#include <string>
#include <octf/utils/Log.h>

namespace octf {

class NodePluginShadow;
class RpcProxy;

/**
 * @brief Utility calls for controlling execution of protocol buffer method
 *
 * At the moment following methods are used in the Framework:
 * - Failed - Checks result of method
 * - SetFailed - Set error of method call
 * - ErrorText - Get error message
 */
class RpcControllerImpl : public google::protobuf::RpcController {
public:
    /**
     * Constructs an RpcController object with no reference to specific
     * communication node.
     *
     * Especially, StartCancel() method has no effect.
     */
    RpcControllerImpl();

    /**
     * @brief Constructs an RpcController object for specified node.
     *
     * It enables calling StartCancel() method and stopping
     * execution of method from client side.
     */
    RpcControllerImpl(NodePluginShadow *node);
    virtual ~RpcControllerImpl() = default;

    /**
     * @deprecated
     */
    void Reset() override;

    /**
     * @brief Check result of method call
     *
     * @retval True indicates error of method call
     * @retval False no error, method finished with success
     */
    bool Failed() const override;

    /**
     * @brief Get error message
     *
     * @return Error message
     */
    std::string ErrorText() const override;

    /**
     * Cancels the method most recently sent with this RpcController.
     *
     * Depending on progress of method, it can have different impact on its
     * execution.
     * On client side method will fail after this call (Failed() method would
     * return true). However, there is no guarantee that such method has not
     * been completed on the server side.
     */
    void StartCancel() override;

    /**
     * @brief Set error on method
     *
     * @param reason Error message
     */
    void SetFailed(const std::string &reason) override;

    /**
     * @brief Checks if StartCancel() was executed for controlled method.
     */
    bool IsCanceled() const override;

    /**
     * @deprecated
     */
    void NotifyOnCancel(google::protobuf::Closure *callback) override;

    /**
     * @brief Gets output stream for speciifed severity
     *
     * @param severity Output stream severity
     *
     * @return Reference to output stream
     */
    virtual log::OutputStream &getOutputStream(log::Severity severity);

private:
    /**
     * Error message
     */
    std::string m_err;

    bool m_cancelled;

    std::shared_ptr<RpcProxy> m_rpcProxy;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_RPCCONTROLLERIMPL_H
