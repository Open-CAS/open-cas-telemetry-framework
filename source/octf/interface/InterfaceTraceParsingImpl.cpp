/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/interface/InterfaceTraceParsingImpl.h>
#include <octf/trace/parser/IoTraceEventHandlerCsvPrinter.h>

#include <octf/trace/parser/IoTraceEventHandlerJsonPrinter.h>
#include <octf/trace/parser/ParsedIoTraceEventHandlerPrinter.h>
#include <octf/utils/Exception.h>

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
                IoTraceEventHandlerJsonPrinter parser(request->tracepath());
                parser.processEvents();
            } else if (request->format() == proto::OutputFormat::CSV) {
                IoTraceEventHandlerCsvPrinter parser(request->tracepath());
                parser.processEvents();
            } else {
                throw Exception("Invalid output format");
            }
        } else {
            ParsedIoTraceEventHandlerPrinter parser(request->tracepath(),
                                                    request->format());
            parser.processEvents();
        }
    } catch (const Exception &ex) {
        controller->SetFailed(ex.what());
    }

    done->Run();
}

}  // namespace octf
