/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H
#define SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H

#include <memory>

#include <octf/communication/RpcOutputStream.h>
#include <octf/node/INode.h>
#include <octf/proto/InterfaceTraceParsing.pb.h>
#include <octf/proto/trace.pb.h>

namespace octf {

class InterfaceTraceParsingImpl : public proto::InterfaceTraceParsing {
public:
    InterfaceTraceParsingImpl() = default;
    virtual ~InterfaceTraceParsingImpl() = default;

    virtual void ParseTrace(::google::protobuf::RpcController *controller,
                            const ::octf::proto::ParseTraceRequest *request,
                            ::octf::proto::Void *response,
                            ::google::protobuf::Closure *done) override;

    virtual void GetTraceStatistics(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::IoStatisticsSet *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetLatencyHistogram(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::IoHistogramSet *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetLbaHistogram(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetLbaHistogramRequest *request,
            ::octf::proto::IoHistogramSet *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetSizeHistogram(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::IoHistogramSet *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetQueueDepthHistogram(
            ::google::protobuf::RpcController *consroller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::IoHistogramSet *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetFileSystemStatistics(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::FilesystemStatistics *response,
            ::google::protobuf::Closure *done) override;

    virtual void GetDeviceList(
            ::google::protobuf::RpcController *controller,
            const ::octf::proto::GetTraceStatisticsRequest *request,
            ::octf::proto::ListDevicesResponse *response,
            ::google::protobuf::Closure *done) override;

private:
    void printHistogramCsv(::octf::RpcOutputStream &cout,
                           const ::octf::proto::IoHistogramSet *histogramSet);
};

}  // namespace octf

#endif  // SOURCE_OCTF_INTERFACE_INTERFACETRACEPARSINGIMPL_H
