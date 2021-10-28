/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TESTS_OCTF_TRACE_TRACEUTILSTEST_H
#define TESTS_OCTF_TRACE_TRACEUTILSTEST_H

#include <gtest/gtest.h>
#include <third_party/safestringlib.h>
#include <limits>
#include <memory>
#include <octf/octf.h>

class TestTrace {
public:
    constexpr static uint32_t IO_QUEUE_COUNT = 1;
    typedef std::list<octf::proto::trace::ParsedEvent> IoList;

    TestTrace(uint32_t eventNumber)
            : m_pluginSrv("Test", IO_QUEUE_COUNT)
            , m_pluginClnt()
            , m_traceSummary()
            , m_ioList() {
        m_pluginSrv.createInterface<octf::InterfaceTraceManagementImpl>("Test");
        m_pluginSrv.init();
        m_pluginClnt.init();

        removeTraces();
        startTracing();
        fillTrace(eventNumber);
        stopTracing();
    }

    virtual ~TestTrace() {
        removeTraces();
    }

    const octf::proto::TraceSummary &getTraceSummary() const {
        return m_traceSummary;
    }

    IoList &getIoList() {
        return m_ioList;
    }

private:
    void startTracing() {
        octf::Call<octf::proto::StartTraceRequest, octf::proto::Void> call(
                &m_pluginClnt);

        auto input = call.getInput().get();
        input->set_circbuffersize(1);
        input->set_maxsize(100);
        input->set_maxduration(100);

        m_pluginClnt.getTraceCreatingInterface()->StartTracing(
                &call, call.getInput().get(), call.getOutput().get(), &call);

        std::chrono::milliseconds timeout(1000);
        call.waitFor(timeout);
        if (call.Failed()) {
            throw octf::Exception("Cannot start tracing, " + call.ErrorText());
        }

        // Wait for trace running
        octf::proto::TraceState traceState;
        uint32_t retry = 10;
        do {
            octf::Call<octf::proto::Void, octf::proto::TraceSummary> sCall(
                    &m_pluginClnt);
            m_pluginClnt.getTraceCreatingInterface()->GetTraceSummary(
                    &sCall, sCall.getInput().get(), sCall.getOutput().get(),
                    &sCall);

            sCall.waitFor(timeout);
            if (sCall.Failed()) {
                throw octf::Exception("Cannot get tracing state, " +
                                      call.ErrorText());
            }

            if (!retry) {
                throw octf::Exception("Cannot run tracing");
            }

            traceState = sCall.getOutput().get()->state();

            if (octf::proto::TraceState::RUNNING != traceState) {
                std::chrono::milliseconds sleppTime(100);
                std::this_thread::sleep_for(sleppTime);
                retry--;
            }
        } while (octf::proto::TraceState::RUNNING != traceState);
    }

    void stopTracing() {
        octf::Call<const octf::proto::Void, octf::proto::TraceSummary> call(
                &m_pluginClnt);
        m_pluginClnt.getTraceCreatingInterface()->StopTracing(
                &call, call.getInput().get(), call.getOutput().get(), &call);

        std::chrono::milliseconds timeout(1000);
        call.waitFor(timeout);
        if (call.Failed()) {
            throw octf::Exception("Cannot stop tracing, " + call.ErrorText());
        }

        m_traceSummary = *call.getOutput().get();
    }

    void removeTraces() {
        octf::Call<octf::proto::RemoveTracesRequest, octf::proto::TraceList>
                call(&m_pluginClnt);

        auto input = call.getInput().get();
        input->set_prefix("*");
        input->set_force(true);

        m_pluginClnt.getTraceManagementInterface()->removeTraces(
                &call, call.getInput().get(), call.getOutput().get(), &call);

        std::chrono::milliseconds timeout(1000);
        call.waitFor(timeout);
        // Don't check call status, validation of trace removal is done bellow
        // by listing traces and expecting no traces

        octf::Call<octf::proto::TracePathPrefix, octf::proto::TraceList>
                lstCall(&m_pluginClnt);

        auto lstInput = lstCall.getInput().get();
        lstInput->set_prefix("*");

        m_pluginClnt.getTraceManagementInterface()->listTraces(
                &lstCall, lstCall.getInput().get(), lstCall.getOutput().get(),
                &lstCall);

        lstCall.waitFor(timeout);
        if (lstCall.Failed()) {
            throw octf::Exception("Cannot list traces, " + call.ErrorText());
        }

        if (lstCall.getOutput().get()->trace_size()) {
            throw octf::Exception("Cannot remove all traces");
        }
    }

    void fillTrace(uint32_t eventNumber) {
        if (!eventNumber) {
            return;
        }

        // Inserting IO
        for (uint32_t i = 1; i <= eventNumber; i++) {
            struct iotrace_event io;
            iotrace_event_init_hdr(&io.hdr, iotrace_event_type_io, i, i,
                                   sizeof(io));

            io.lba = rand() % std::numeric_limits<decltype(io.lba)>::max();
            io.len = rand() % 256;
            io.operation = rand() % 2 ? iotrace_event_operation_rd
                                      : iotrace_event_operation_wr;

            m_pluginSrv.push(0, &io, sizeof(io));
            pushIo(io);
        }
    }

    void pushIo(const struct iotrace_event &io) {
        octf::proto::trace::ParsedEvent event;

        auto hdr = event.mutable_header();
        hdr->set_sid(io.hdr.sid);
        hdr->set_timestamp(io.hdr.timestamp);

        auto ioEvent = event.mutable_io();
        ioEvent->set_lba(io.lba);
        ioEvent->set_len(io.len);
        if (iotrace_event_operation_rd == io.operation) {
            ioEvent->set_operation(octf::proto::trace::IoType::Read);
        } else {
            ioEvent->set_operation(octf::proto::trace::IoType::Write);
        }

        m_ioList.emplace_back(event);
    }

    class IOTracePluginShadow : public octf::NodePluginShadow {
    public:
        IOTracePluginShadow()
                : octf::NodePluginShadow(octf::NodeId("Test")) {}
        virtual ~IOTracePluginShadow() = default;

        bool initCustom() override {
            if (!createInterface<octf::proto::InterfaceTraceCreating_Stub>()) {
                throw octf::Exception(
                        "Error creating Trace Creating interface");
            }

            if (!createInterface<
                        octf::proto::InterfaceTraceManagement_Stub>()) {
                throw octf::Exception(
                        "Error creating Trace Creating interface");
            }

            return true;
        }

        std::shared_ptr<octf::proto::InterfaceTraceCreating>
        getTraceCreatingInterface() {
            return findInterface<octf::proto::InterfaceTraceCreating_Stub>();
        }

        std::shared_ptr<octf::proto::InterfaceTraceManagement>
        getTraceManagementInterface() {
            return findInterface<octf::proto::InterfaceTraceManagement_Stub>();
        }
    };

private:
    octf::IOTracePlugin m_pluginSrv;
    IOTracePluginShadow m_pluginClnt;
    octf::proto::TraceSummary m_traceSummary;
    IoList m_ioList;
};

#endif  // TESTS_OCTF_TRACE_TRACEUTILSTEST_H
