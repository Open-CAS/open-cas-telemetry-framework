/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/interface/InterfaceConfigurationImpl.h>

#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/internal/FrameworkConfigurationInternal.h>

namespace octf {

void InterfaceConfigurationImpl::getTraceRepositoryPath(
        ::google::protobuf::RpcController *,
        const ::octf::proto::Void *,
        ::octf::proto::TraceRepositoryPath *response,
        ::google::protobuf::Closure *done) {
    response->set_path(getFrameworkConfiguration().getTraceDir());
    done->Run();
}

void InterfaceConfigurationImpl::setTraceRepositoryPath(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::TraceRepositoryPath *request,
        ::octf::proto::Void *,
        ::google::protobuf::Closure *done) {
    try {
        if (request->path() == "") {
            throw Exception("Empty trace repository path");
        }

        using namespace fsutils;

        if (!checkPermissions(request->path(), PermissionType::Execute) ||
            !checkPermissions(request->path(), PermissionType::Read)) {
            throw Exception("No access to trace directory");
        }

        auto &config = proto::getFrameworkConfiguration();
        config.mutable_paths()->set_trace(request->path());
        proto::saveFrameworkConfiguration();

    } catch (Exception &e) {
        controller->SetFailed(e.getMessage());
    }

    done->Run();
}

}  // namespace octf
