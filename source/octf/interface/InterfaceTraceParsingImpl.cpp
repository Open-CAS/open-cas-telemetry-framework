/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceTraceParsingImpl.h>

#include <octf/communication/RpcOutputStream.h>
#include <octf/trace/parser/IoTraceEventHandlerCsvPrinter.h>
#include <octf/trace/parser/IoTraceEventHandlerJsonPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerStatistics.h>
#include <octf/utils/Exception.h>
#include <octf/utils/table/Table.h>

namespace octf {

void InterfaceTraceParsingImpl::ParseTrace(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::ParseTraceRequest *request,
        ::octf::proto::Void *response,
        ::google::protobuf::Closure *done) {
    (void) (response);

    try {
        if (request->raw()) {
            if (request->format() == proto::OutputFormat::JSON) {
                IoTraceEventHandlerJsonPrinter handler(request->tracepath());
                handler.processEvents();
            } else if (request->format() == proto::OutputFormat::CSV) {
                IoTraceEventHandlerCsvPrinter handler(request->tracepath());
                handler.processEvents();
            } else {
                throw Exception("Invalid output format");
            }
        } else {
            ParsedIoTraceEventHandlerPrinter handler(request->tracepath(),
                                                     request->format());
            handler.processEvents();
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void InterfaceTraceParsingImpl::GetTraceStatistics(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::IoStatisticsSet *response,
        ::google::protobuf::Closure *done) {
    try {
        ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
        handler.processEvents();
        handler.getStatisticsSet().getIoStatisticsSet(response);

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);

            cout << log::reset;

            table::Table table;
            table::setHeader(table[0], &response->statistics(0));

            int count = response->statistics_size();
            for (int i = 0; i < count; i++) {
                table[i + 1] << response->statistics(i);
            }

            cout << table << std::endl;
            // The CSV output was requested, to prevent print response in JSON
            // format disable caller output
            cout << log::disable;
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
