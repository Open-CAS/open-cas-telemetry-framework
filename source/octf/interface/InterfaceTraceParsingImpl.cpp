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

            int count = response->statistics_size();
            for (int i = 0; i < count; i++) {
                table::setHeader(table[0], &response->statistics(i));
                table[i + 1] << response->statistics(i);
            }

            cout << table << std::endl;
            // The CSV output was requested, to prevent printing response in
            // JSON format disable caller output
            cout << log::disable;
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void octf::InterfaceTraceParsingImpl::GetLatencyHistogram(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::IoHistogramSet *response,
        ::google::protobuf::Closure *done) {
    try {
        ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
        handler.processEvents();
        handler.getStatisticsSet().getIoLatencyHistogramSet(response);

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);

            cout << log::reset;

            table::Table table;
            auto &hdr = table[0];

            auto setter = [&table](const std::string &operation,
                                   const proto::IoStatisticsDescription &desc,
                                   const proto::Histogram &entry) {
                auto &row = table[table.size()];

                row << desc;
                row["operation"] = operation;
                for (int i = 0; i < entry.range_size(); i++) {
                    auto end = entry.range(i).end();
                    auto count = entry.range(i).count();
                    row[std::to_string(end)] = count;
                }
            };

            int count = response->histogram_size();
            for (int i = 0; i < count; i++) {
                const auto &histogram = response->histogram(i);
                const auto &desc = histogram.desc();

                setter("read", desc, histogram.read());
                setter("write", desc, histogram.write());
                setter("discard", desc, histogram.discard());
                setter("flush", desc, histogram.flush());
                setter("total", desc, histogram.total());
            }

            hdr.setupHeader();
            cout << table << std::endl;
            // The CSV output was requested, to prevent printing response in
            // JSON format disable caller output
            cout << log::disable;
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

// TODO (tomaszrybicki) Separate this code to function
// TODO (tomaszrybicki) Consider different CSV structure
void InterfaceTraceParsingImpl::GetLbaHistogram(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetLbaHistogramRequest *request,
        ::octf::proto::IoHistogramSet *response,
        ::google::protobuf::Closure *done) {
    try {
        ParsedIoTraceEventHandlerStatistics handler(request->tracepath(),
                                                    request->bucketsize());
        handler.processEvents();
        handler.getStatisticsSet().getIoLbaHistogramSet(response);

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);

            cout << log::reset;

            table::Table table;
            auto &hdr = table[0];

            auto setter = [&table](const std::string &operation,
                                   const proto::IoStatisticsDescription &desc,
                                   const proto::Histogram &entry) {
                auto &row = table[table.size()];

                row << desc;
                row["operation"] = operation;
                for (int i = 0; i < entry.range_size(); i++) {
                    auto end = entry.range(i).end();
                    auto count = entry.range(i).count();
                    row[std::to_string(end)] = count;
                }
            };

            int count = response->histogram_size();
            for (int i = 0; i < count; i++) {
                const auto &histogram = response->histogram(i);
                const auto &desc = histogram.desc();

                setter("read", desc, histogram.read());
                setter("write", desc, histogram.write());
                setter("discard", desc, histogram.discard());
                setter("total", desc, histogram.total());
            }

            hdr.setupHeader();
            cout << table << std::endl;
            // The CSV output was requested, to prevent printing response in
            // JSON format disable caller output
            cout << log::disable;
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
