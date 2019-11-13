/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceTraceParsingImpl.h>

#include <google/protobuf/util/json_util.h>
#include <iostream>
#include <octf/communication/RpcOutputStream.h>
#include <octf/trace/parser/IoTraceEventHandlerCsvPrinter.h>
#include <octf/trace/parser/IoTraceEventHandlerJsonPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerStatistics.h>
#include <octf/trace/parser/TraceEventHandlerFilesystemStatistics.h>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
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
            printHistogramCsv(cout, response);

            // The CSV output was requested, to prevent printing response in
            // JSON format disable caller output
            cout << log::disable;
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void InterfaceTraceParsingImpl::GetLbaHistogram(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetLbaHistogramRequest *request,
        ::octf::proto::IoHistogramSet *response,
        ::google::protobuf::Closure *done) {
    try {
        uint64_t bucketSize;
        if (request->bucketsize() != 0) {
            bucketSize = request->bucketsize();
        } else {
            bucketSize = ParsedIoTraceEventHandlerStatistics::
                    DEFAULT_LBA_HIT_MAP_RANGE_SIZE;
        }

        ParsedIoTraceEventHandlerStatistics handler(request->tracepath(),
                                                    bucketSize);

        if (request->subrangestart() || request->subrangeend()) {
            if (request->subrangestart() < request->subrangeend() &&
                request->subrangestart() >= 0) {
                handler.setExclusiveSubrange(request->subrangestart(),
                                             request->subrangeend());
            } else {
                throw Exception("Invalid values given for subrange");
            }
        }

        handler.enableLbaHistogram();
        handler.processEvents();
        handler.getStatisticsSet().getIoLbaHistogramSet(response);

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);
            printHistogramCsv(cout, response);

            // The CSV output was requested, to prevent printing histogramSet in
            // JSON format disable caller output
            cout << log::disable;
        }

    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void InterfaceTraceParsingImpl::printHistogramCsv(
        ::octf::RpcOutputStream &cout,
        const ::octf::proto::IoHistogramSet *histogramSet) {
    // Lamba to set histogram entries in table
    auto setter = [](const std::string &operation,
                     const proto::IoStatisticsDescription &desc,
                     const proto::Histogram &entry, table::Table &table) {
        for (int i = 0; i < entry.range_size(); i++) {
            auto begin = entry.range(i).begin();
            auto end = entry.range(i).end();
            auto count = entry.range(i).count();

            // Index rows by begin value of range to keep them sorted
            auto &row = table[begin];
            row << desc;

            row["rangeBegin"] = begin;
            row["rangeEnd"] = end;
            row[operation] = count;
        }
    };

    // Keep table per each device
    std::list<table::Table> tables;

    int count = histogramSet->histogram_size();
    for (int i = 0; i < count; i++) {
        // Create a new table for each device
        tables.emplace_back();
        table::Table &table = tables.back();

        // Setup CSV header and column names association
        auto &hdr = table[0];
        table::setHeader(hdr, &histogramSet->histogram(i).desc());
        hdr["rangeBegin"] = "rangeBegin";
        hdr["rangeEnd"] = "rangeEnd";
        hdr["read"] = "read";
        hdr["write"] = "write";
        hdr["discard"] = "discard";
        hdr["total"] = "total";

        const auto &histogram = histogramSet->histogram(i);
        const auto &desc = histogram.desc();

        setter("read", desc, histogram.read(), table);
        setter("write", desc, histogram.write(), table);
        setter("discard", desc, histogram.discard(), table);
        setter("total", desc, histogram.total(), table);

        // Only first table shall contain a header
        if (i == 0) {
            hdr.setupHeader();
        } else {
            hdr.clear();
        }
    }

    cout << log::reset;
    for (auto &table : tables) {
        cout << table << std::endl;
    }
}

void InterfaceTraceParsingImpl::GetFileSystemStatistics(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::FilesystemStatistics *response,
        ::google::protobuf::Closure *done) {
    try {
        TraceEventHandlerFilesystemStatistics handler(request->tracepath());
        handler.processEvents();
        handler.getFilesystemStatistics(response);

        RpcOutputStream cout(log::Severity::Information, controller);
        cout << log::reset;

        if (request->format() == proto::OutputFormat::CSV) {
            table::Table table;
            auto &hdr = table[0];

            // Set header
            for (int i = 0; i < response->entries_size(); i++) {
                table::setHeader(hdr, &response->entries(i));
            }

            // Set rows
            for (int i = 1; i <= response->entries_size(); i++) {
                table[i] << response->entries(i - 1);
            }

            hdr.setupHeader();
            cout << table << std::endl;
        } else if (request->format() == proto::OutputFormat::JSON) {
            google::protobuf::util::JsonOptions jsonOptions;
            std::string output;

            jsonOptions.always_print_primitive_fields = false;
            jsonOptions.add_whitespace = true;

            google::protobuf::util::MessageToJsonString(*response, &output,
                                                        jsonOptions);

            cout << output << std::endl;

        } else {
            throw Exception("Invalid output format");
        }

        cout << log::disable;

    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
