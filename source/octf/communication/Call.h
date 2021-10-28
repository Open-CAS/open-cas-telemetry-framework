/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_COMMUNICATION_CALL_H
#define SOURCE_OCTF_COMMUNICATION_CALL_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <octf/communication/RpcControllerImpl.h>
#include <octf/utils/Semaphore.h>

namespace octf {

constexpr std::chrono::milliseconds CALL_DEFAULT_TIMEOUT =
        std::chrono::milliseconds(5000);

class NodePluginShadow;
/**
 * @brief Very basic utility class for calling methods from an interface.
 *
 * This class contains input type and output type for call. Specified as
 * template parameters. In addition it implements closure and RPC controller
 * interfaces which are required for protocol buffer call. Closure interface
 * notifies about method completion. While RPC controller is used for signaling
 * error of method
 *
 * @note To check call result, call Failed method (RpcController::Failed)
 *
 *
 * @code
 *
 * // An example of use this class for performing protocol buffer call
 *
 * Call<proto::Void, proto::NodeId> call; // Defining call object
 *
 * call.getInput().... // Setting input parameters
 *
 * plugin->getIdentificationInterface()->getNodeId(&call, &call.getInput(),
 *      &call.getOutput(), &call); // Protocol buffer call
 *
 * call.wait(); // Wait for response
 *
 * if (call.Failed()) {
 *      // method call error
 * } else {
 *      // call successful
 *       call.getOutput().... // Read output parameters
 * }
 *
 * @endcode
 *
 * @tparam InputType Input type for method call
 * @tparam OutputType Output type of method
 */
template <class InputType, class OutputType>
class Call : public google::protobuf::Closure, public RpcControllerImpl {
public:
    /**
     * @brief Constructs call for specified node.
     *
     * It enables calling StartCancel() method and stopping
     * execution of method from client side.
     */
    Call(NodePluginShadow *node)
            : Closure()
            , RpcControllerImpl(node)
            , m_cmpl()
            , m_input(std::make_shared<InputType>())
            , m_output(std::make_shared<OutputType>())
            , m_done(false) {}

    Call(std::shared_ptr<InputType> in,
         std::shared_ptr<OutputType> out,
         NodePluginShadow *node)
            : Closure()
            , RpcControllerImpl(node)
            , m_cmpl()
            , m_input(in)
            , m_output(out)
            , m_done(false) {}

    Call(std::shared_ptr<InputType> in, std::shared_ptr<OutputType> out)
            : Closure()
            , RpcControllerImpl()
            , m_cmpl()
            , m_input(in)
            , m_output(out)
            , m_done(false) {}

    virtual ~Call() {
        // TODO: Remove from Connection Context
        if (!m_done) {
            StartCancel();
        }
    }

    /**
     * @brief Implementation of required method for Closure class.
     *
     * Implements abstract method from google::protobuf::Closure class.
     * Is called by every sent method on its completion.
     *
     * Call of this method does not indicate if method was executed
     * successfully. To check it, Failed() method (from RpcController interface)
     * should be called.
     */
    void Run() override {
        m_done = true;
        m_cmpl.post();
    }

    /**
     * Waits for method response
     */
    void wait() {
        m_cmpl.wait();
    }

    /**
     *  @brief Waits for method reponse for specified time
     *
     *  @retval true Response received before timeout
     *  @retval false Timeout reached
     */
    bool waitFor(std::chrono::milliseconds timeout = CALL_DEFAULT_TIMEOUT) {
        bool result = m_cmpl.waitFor(timeout);
        if ((!result) && (!m_done)) {
            StartCancel();
            // Closure Run() is expected to be call internally in StartCancel()
            // so no need to call it here explicitly

            SetFailed("Method call timeout.");
        }
        return result;
    }

    /**
     * @brief Gets Input parameter
     *
     * @return Input parameter
     */
    std::shared_ptr<InputType> getInput() {
        return m_input;
    }

    /**
     * @brief Gets Output parameter
     *
     * @return Output parameter
     */
    std::shared_ptr<OutputType> getOutput() {
        return m_output;
    }

private:
    /**
     * Semaphore for signaling end of interface method call
     */
    Semaphore m_cmpl;

    /**
     * Input parameter
     */
    std::shared_ptr<InputType> m_input;

    /**
     * Output parameter
     */
    std::shared_ptr<OutputType> m_output;

    std::atomic<bool> m_done;
};

/**
 * @typedef Call with generic input and output type,
 *  can be used when these types are unknown on compilation.
 */
typedef Call<google::protobuf::Message, google::protobuf::Message> CallGeneric;

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_CALL_H
