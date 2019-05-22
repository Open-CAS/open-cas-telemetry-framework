/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <dirent.h>
#include <google/protobuf/util/json_util.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

ProtobufReaderWriter::ProtobufReaderWriter(const std::string &filePath)
        : m_filePath(filePath)
        , m_directoryPath("") {
    std::size_t dirEndPos = filePath.rfind('/');

    if (dirEndPos == 0) {
        m_directoryPath = "/";
    } else if (dirEndPos != std::string::npos) {
        m_directoryPath = filePath.substr(0, dirEndPos);
    } else {
        m_directoryPath = ".";
    }
}

bool ProtobufReaderWriter::read(google::protobuf::Message &message) {
    // 1. Open and read file
    std::ifstream file;
    file.open(m_filePath);
    if (!file.good()) {
        // Cannot open file
        return false;
    }

    // 2. Read all file content
    std::stringstream ss;
    ss << file.rdbuf();

    // 3. Check if read performed well
    if (!file.good()) {
        // cannot read file
        file.close();
        return false;
    }

    // File no needed any more, close it
    file.close();

    // 4. Parse JSON string into procol buffer message
    google::protobuf::util::Status status;
    google::protobuf::util::JsonParseOptions opts;

    // We want to have exact fields, when unknown one then we except error
    opts.ignore_unknown_fields = false;

    status = google::protobuf::util::JsonStringToMessage(ss.str(), &message,
                                                         opts);

    return status.ok();
}

bool ProtobufReaderWriter::write(const google::protobuf::Message &message) {
    // 1. First generate string with configuration

    google::protobuf::util::Status status;
    google::protobuf::util::JsonPrintOptions opts;
    std::string str;

    // enable new lines in output for each field
    opts.add_whitespace = true;

    // Default printing configuration, when field has default value
    // (e.g. a number field set to 0), it's not printed, but we want to have
    // this field in output, enable this
    opts.always_print_primitive_fields = true;

    status = google::protobuf::util::MessageToJsonString(message, &str, opts);
    if (!status.ok()) {
        return false;
    };

    // 2. Save configuration to the configuration file

    std::ofstream file(m_filePath);

    file << str;
    if (!file.good()) {
        // Cannot save the file
        file.close();
        return false;
    }

    // 3. Make configuration file persistent

    file.flush();
    if (!file.good()) {
        // Cannot synchronize the file
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool ProtobufReaderWriter::isFileAvailable(std::fstream::openmode access) {
    struct stat _stat;
    DIR *dir = ::opendir(m_directoryPath.c_str());
    if (!dir) {
        std::stringstream msg;

        msg << "Cannot access directory: " << m_directoryPath << "ERROR("
            << errno << ") " << strerror(errno);

        throw Exception(msg.str());
    }
    closedir(dir);
    dir = NULL;

    // Use permission defines from dirent.h
    int accessDirent = F_OK;
    if (access & std::fstream::in) {
        accessDirent = accessDirent | R_OK;
    }
    if (access & std::fstream::out) {
        accessDirent = accessDirent | W_OK;
    }

    int result = ::stat(m_filePath.c_str(), &_stat);
    if (0 == result) {
        if (S_ISREG(_stat.st_mode)) {
            // Check there is read/write access to the file
            result = ::access(m_filePath.c_str(), accessDirent);
            if (result != 0) {
                return false;
            }

            return true;

        } else {
            // We expect regular file but it's not
            throw Exception("Expecting regular file, but it's not, " +
                            m_filePath);
        }
    } else if (ENOENT == errno) {
        // File doesn't exist
        return false;
    }

    std::stringstream msg;
    msg << "Cannot access file: " << m_filePath << "ERROR(" << errno << ") "
        << strerror(errno);
    throw Exception(msg.str());

    return false;
}

bool ProtobufReaderWriter::remove() {
    bool result;
    try {
        if (isFileAvailable()) {
            result = ::remove(m_filePath.c_str());
            return (result == 0);
        } else {
            return true;
        }
    } catch (Exception &e) {
        // TODO log error
        log::cerr << e.what();
    }

    return false;
}

}  // namespace octf
