/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/communication/RpcOutputStream.h>

#include <octf/communication/RpcControllerImpl.h>
#include <octf/utils/Exception.h>

namespace octf {

RpcOutputStream::RpcOutputStream(
        log::Severity severity,
        ::google::protobuf::RpcController *rpcController) {
    auto rpcCtrlImpl = dynamic_cast<RpcControllerImpl *>(rpcController);
    if (!rpcCtrlImpl) {
        throw Exception("Cannot get output stream of RPC Controller");
    }

    m_os = &rpcCtrlImpl->getOutputStream(severity);
}

RpcOutputStream::~RpcOutputStream() {}

}  // namespace octf
