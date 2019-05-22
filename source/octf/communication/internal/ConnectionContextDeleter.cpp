/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/communication/internal/ConnectionContextDeleter.h>

using namespace std;

namespace octf {

ConnectionContextDeleter::ConnectionContextDeleter()
        : m_inactiveConnCntxList()
        , m_thread()
        , m_run(true) {
    m_thread = std::thread([this]() {
        while (m_run) {
            m_sem.wait();
            empty();
        }

        empty();
    });
}

ConnectionContextDeleter::~ConnectionContextDeleter() {
    m_run = false;
    m_sem.post();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void ConnectionContextDeleter::addConnectionContext(
        ConnectionContextShRef &&cntx) {
    std::lock_guard<std::mutex> lock(m_lock);
    m_inactiveConnCntxList.push_back(std::move(cntx));
    m_sem.post();
}

void ConnectionContextDeleter::empty() {
    ConnectionContextShRef conn;
    while (getConnection(conn)) {
    };
}

bool ConnectionContextDeleter::getConnection(ConnectionContextShRef &conn) {
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_inactiveConnCntxList.size()) {
        conn = *m_inactiveConnCntxList.begin();
        m_inactiveConnCntxList.pop_front();
        return true;
    } else {
        return false;
    }
}
}  // namespace octf
