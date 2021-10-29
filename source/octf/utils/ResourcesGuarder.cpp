/*
 * Copyright(c) 2012-2020 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#include <list>
#include <mutex>
#include <sstream>
#include <thread>
#include <octf/utils/FileOperations.h>
#include <octf/utils/Log.h>
#include <octf/utils/ResourcesGuarder.h>

namespace octf {

class ResourcesGuarder::Controller {
public:
    Controller()
            : m_mutex()
            , m_clients(0) {}
    virtual ~Controller() {}

    void lock(double utilization) {
        std::unique_lock<std::mutex> guard(m_mutex);

        if (0 == m_clients) {
            // Allow working for first client to avoid starving
            m_clients++;
            return;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));

        while (isLimit(utilization)) {
            // Many threads fight for resources, so if one got it, let's wait
            // a few seconds for its consumption
            guard.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            guard.lock();

            if (0 == m_clients) {
                // Allow working for first client to avoid starving
                break;
            }
        }

        m_clients++;
    }

    void unlock() {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_clients) {
            m_clients--;
        } else {
            throw Exception("Resource guarder ERROR, client unlock error");
        }
    }

    bool isLimit(double utilization) {
        if (isMemoryLimit(utilization)) {
            return true;
        }

        if (isOpenFilesLimit(utilization)) {
            return true;
        }

        return false;
    }

    uint64_t getMemoryAvailable() {
        try {
            return tryGetMemory("MemAvailable");
        } catch (Exception &e) {
            // Try to backup with system information

            struct sysinfo si;

            if (sysinfo(&si)) {
                throw Exception(
                        "Resource guarder ERROR, cannot get amount of "
                        "available memory");
            }

            return si.freeram;
        }
    }

    uint64_t getMemoryTotal() {
        try {
            return tryGetMemory("MemTotal");
        } catch (Exception &e) {
            // Try to backup with system information

            struct sysinfo si;

            if (sysinfo(&si)) {
                throw Exception(
                        "Resource guarder ERROR, cannot get amount of total "
                        "memory");
            }

            return si.totalram;
        }
    }

    bool isMemoryLimit(double utilization) {
        double total = getMemoryTotal();
        double avail = getMemoryAvailable();
        double used = 1.0;

        if (avail < total) {
            used = (total - avail) / avail;
        }

        return used >= utilization;
    }

    uint64_t getOpenFilesNumber() {
        auto pid = getpid();
        std::string path = "/proc/" + std::to_string(pid) + "/fd";

        std::list<std::string> fds;
        fsutils::readDirectoryContents(path, fds, fsutils::FileType::Any);

        return fds.size();
    }

    uint64_t getOpenFilesLimit() {
        struct rlimit limit;

        if (::getrlimit(RLIMIT_NOFILE, &limit) || 0 == limit.rlim_cur) {
            throw Exception(
                    "Resource guarder ERROR, cannot get limit of open files");
        }

        return limit.rlim_cur;
    }

    bool isOpenFilesLimit(double utilization) {
        double open = getOpenFilesNumber();
        double limit = getOpenFilesLimit();
        double used = open / limit;

        return used >= utilization;
    }

private:
    uint64_t tryGetMemory(const std::string &type) {
        constexpr auto path = "/proc/meminfo";

        std::ifstream file(path);
        if (!file.good()) {
            throw Exception("Resource guarder ERROR, cannot open file: " +
                            std::string(path));
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream is(line);
            is.imbue(std::locale::classic());

            std::string what;
            uint64_t amount;
            std::string unit;

            is >> what;

            if (what != (type + ":")) {
                continue;
            }

            is >> amount;
            if (is.fail()) {
                throw Exception(
                        "Resource guarder ERROR, cannot parse amount of "
                        "available memory");
            }

            is >> unit;
            if (unit != "kB") {
                throw Exception(
                        "Resource guarder ERROR, available memory unit "
                        "Mismatch");
            }

            amount *= 1024ULL;
            return amount;
        }

        throw Exception(
                "Resource guarder ERROR, cannot find available memory entry");
    }

private:
    std::mutex m_mutex;
    uint64_t m_clients;
};

ResourcesGuarder::ResourcesGuarder(double utilization)
        : m_locked(false)
        , m_utilization(utilization) {
    if (m_utilization > 1.0) {
        throw Exception("Resource guarder ERROR, invalid utilization value");
    }
}

ResourcesGuarder::~ResourcesGuarder() {
    if (m_locked) {
        getController().unlock();
    }
}

void ResourcesGuarder::lock() {
    if (m_locked) {
        throw Exception("Resource guarder ERROR, dead lock");
    }

    m_locked = true;
    getController().lock(m_utilization);
}

void ResourcesGuarder::unlock() {
    if (!m_locked) {
        throw Exception("Resource guarder ERROR, unlocking not locked");
    }

    getController().unlock();
    m_locked = false;
}

ResourcesGuarder::Controller &ResourcesGuarder::getController() {
    static Controller ctrl;

    return ctrl;
}

}  // namespace octf
