/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/interface/InterfaceTraceParsingImpl.h>

#include <google/protobuf/util/json_util.h>
#include <iostream>
#include <memory>
#include <ostream>
#include <set>
#include <octf/communication/RpcOutputStream.h>
#include <octf/trace/TraceLibrary.h>
#include <octf/trace/parser/HandlerRunner.h>
#include <octf/trace/parser/IoTraceEventHandlerCsvPrinter.h>
#include <octf/trace/parser/IoTraceEventHandlerJsonPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerExtensionBuilder.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerStatistics.h>
#include <octf/trace/parser/TraceEventHandlerDevicesList.h>
#include <octf/trace/parser/TraceEventHandlerFilesystemStatistics.h>
#include <octf/trace/parser/TraceEventHandlerWorkset.h>
#include <octf/trace/parser/extensions/LRUExtensionBuilder.h>
#include <octf/trace/parser/extensions/TraceExtensionSet.h>
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
        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read("Statistics", *response)) {
            /* No cached result, perform required processing */
            ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
            handler.processEvents();
            handler.getStatisticsSet().getIoStatisticsSet(response);
            cache.write("Statistics", *response);
        }

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
        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read("LatencyHistogram", *response)) {
            ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
            handler.processEvents();
            handler.getStatisticsSet().getIoLatencyHistogramSet(response);
            cache.write("LatencyHistogram", *response);
        }

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

        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read(*request, *response)) {
            /* No cached result, perform required processing */
            handler.enableLbaHistogram();
            handler.processEvents();
            handler.getStatisticsSet().getIoLbaHistogramSet(response);
            cache.write(*request, *response);
        }

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);
            printHistogramCsv(cout, response);

            // The CSV output was requested, to prevent printing
            // histogramSet in JSON format disable caller output
            cout << log::disable;
        }

    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void InterfaceTraceParsingImpl::GetSizeHistogram(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::IoHistogramSet *response,
        ::google::protobuf::Closure *done) {
    try {
        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read("SizeHistogram", *response)) {
            /* No cached result, perform required processing */
            ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
            handler.processEvents();
            handler.getStatisticsSet().getIoSizeHistogramSet(response);
            cache.write("SizeHistogram", *response);
        }

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

void InterfaceTraceParsingImpl::GetQueueDepthHistogram(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::IoHistogramSet *response,
        ::google::protobuf::Closure *done) {
    try {
        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read("QueueDepthHistogram", *response)) {
            /* No cached result, perform required processing */
            ParsedIoTraceEventHandlerStatistics handler(request->tracepath());
            handler.processEvents();
            handler.getStatisticsSet().getQueueDepthHistogramSet(response);
            cache.write("QueueDepthHistogram", *response);
        }

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
        // Cache response in trace cache
        auto trace = TraceLibrary::get().getTrace(request->tracepath());
        auto &cache = trace->getCache();

        // Try read result from cache
        if (!cache.read("FileSystemStatistics", *response)) {
            /* No cached result, perform required processing */
            TraceEventHandlerFilesystemStatistics handler(request->tracepath());
            handler.processEvents();
            handler.getFilesystemStatistics(response);
            cache.write("FileSystemStatistics", *response);
        }

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
            cout << table << std::endl << log::disable;

        } else if (request->format() != proto::OutputFormat::JSON) {
            throw Exception("Invalid output format");
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void octf::InterfaceTraceParsingImpl::GetDeviceList(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::GetTraceStatisticsRequest *request,
        ::octf::proto::ListDevicesResponse *response,
        ::google::protobuf::Closure *done) {
    try {
        TraceEventHandlerDevicesList handler(request->tracepath());
        handler.processEvents();

        response->Clear();
        handler.getDevicesList(response);

        if (request->format() == proto::OutputFormat::CSV) {
            RpcOutputStream cout(log::Severity::Information, controller);
            cout << log::reset;

            table::Table table;
            auto &hdr = table[0];

            // Set header
            for (int i = 0; i < response->devices_size(); i++) {
                table::setHeader(hdr, &response->devices(i));
            }

            // Set rows
            for (int i = 1; i <= response->devices_size(); i++) {
                table[i] << response->devices(i - 1);
            }

            hdr.setupHeader();
            cout << table << std::endl << log::disable;

        } else if (request->format() != proto::OutputFormat::JSON) {
            throw Exception("Invalid output format");
        }

    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

void InterfaceTraceParsingImpl::BuildExtensions(
        ::google::protobuf::RpcController *controller,
        const ::octf::proto::BuildExtensionsRequest *request,
        ::octf::proto::Void *,
        ::google::protobuf::Closure *done) {
    RpcOutputStream cout(log::Severity::Information, controller);
    RpcOutputStream verbose(log::Severity::Verbose, controller);
    cout << log::reset;

    const auto &tracePath = request->tracepath();
    auto trace = TraceLibrary::get().getTrace(request->tracepath());
    auto &cache = trace->getCache();
    uint64_t workset = 0;

    if (!cache.read("BuildExtensionsWorkset", workset)) {
        verbose << "Get workset for trace " << request->tracepath()
                << std::endl;

        /* No cached result, perform required processing */
        CasTraceEventHandlerWorkset handler(request->tracepath());
        handler.processEvents();
        workset = handler.getWorkset();
        cache.write("BuildExtensionsWorkset", workset);
    }

    using Handler = ParsedIoTraceEventHandlerExtensionBuilder;
    HandlerRunner<Handler> runner;
    std::string result;
    std::list<std::shared_ptr<Handler>> handlers;

    const std::vector<uint64_t> percentages{5, 10, 15, 20, 50, 100};
    for (auto prcnt : percentages) {
        auto factory = [&handlers, tracePath, workset, prcnt]() {
            auto builder =
                    std::make_shared<LRUExtensionBuilder>(workset, prcnt);

            auto handler = std::make_shared<Handler>(tracePath, builder);
            handlers.push_back(handler);

            return handler;
        };
        auto cmpl = [](std::shared_ptr<Handler>) {};
        auto error = [&result](const std::string &error) { result = error; };

        runner.addHandler(factory, cmpl, error);
    }

    runner.run();
    if (!result.empty()) {
        // An error occurred while running handlers by runner
        throw Exception(result);
    }

    TraceExtensionSet eSet;
    for (const auto &hndlr : handlers) {
        eSet.addTraceExtension(hndlr->getTraceExtensions());
    }

    // Define variables for
    std::string jsonTrace;
    google::protobuf::util::JsonOptions jsonOptions;
    jsonOptions.always_print_primitive_fields = true;
    jsonOptions.add_whitespace = false;
    table::Table tab;

    proto::trace::TraceExtension msg;
    proto::trace::TraceExtensionResult feature;
    uint64_t sid = 0;

    // Print header if needed
    if (proto::OutputFormat::CSV == request->format()) {
        auto &hdr = tab[0];

        // Reserver first column for sid
        hdr["sid"] = "sid";

        for (const auto &hndlr : handlers) {
            auto builder = hndlr->getExtensionBuilder();

            const auto &name = builder->getName();
            auto &feature = (*msg.mutable_feature())[name];
            feature.CopyFrom(*builder->getExtensionMessagePrototype());
        }

        table::setHeader(hdr, &msg);
        cout << tab << std::endl;
    }

    while (eSet.getReader().hasNext()) {
        const auto &name = eSet.getName();

        eSet.read(sid, feature);
        msg.set_sid(sid);
        if (feature.ResultType_case()) {
            (*msg.mutable_feature())[name] = feature;
        }

        if (!eSet.hasNext() || eSet.getNextSid() != sid) {
            // time to print
            switch (request->format()) {
            case proto::OutputFormat::CSV: {
                tab[0].clear();
                tab[0] << msg;
                cout << tab << std::endl;
            } break;
            case proto::OutputFormat::JSON: {
                jsonTrace.clear();
                google::protobuf::util::MessageToJsonString(msg, &jsonTrace,
                                                            jsonOptions);
                cout << jsonTrace << std::endl;
            } break;
            default: {
                throw Exception("Invalid output format");
            } break;
            }

            msg.Clear();
        }
    }

    if (proto::OutputFormat::CSV == request->format()) {
        cout << log::disable;
    }

    done->Run();
}

}  // namespace octf
