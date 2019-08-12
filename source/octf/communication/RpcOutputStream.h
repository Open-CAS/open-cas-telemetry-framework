/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_RPCOUTPUTSTREAM_H
#define SOURCE_OCTF_COMMUNICATION_RPCOUTPUTSTREAM_H

#include <google/protobuf/service.h>
#include <octf/utils/Log.h>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief Output stream for RPC
 *
 * It allows to stream output from an interface method to a RPC caller
 */
class RpcOutputStream : public NonCopyable {
public:
    RpcOutputStream(log::Severity severity,
                    ::google::protobuf::RpcController *rpcController);
    virtual ~RpcOutputStream();

    template <typename Type>
    RpcOutputStream &operator<<(const Type &value) {
        (*m_os) << value;
        return *this;
    }

    RpcOutputStream &operator<<(std::ostream &(*fn)(std::ostream &) ) {
        (*m_os) << fn;
        return *this;
    }

private:
    log::OutputStream *m_os;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_RPCOUTPUTSTREAM_H
