/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <google/protobuf/util/json_util.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <string>
#include <octf/utils/Exception.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/Log.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

ProtobufReaderWriter::ProtobufReaderWriter(const std::string &filePath)
        : m_filePath(filePath)
        , m_directoryPath("")
        , m_readFd(-1)
        , m_writeFd(-1)
        , m_errnoMsg("") {
    std::size_t dirEndPos = filePath.rfind('/');

    if (dirEndPos == 0) {
        m_directoryPath = "/";
    } else if (dirEndPos != std::string::npos) {
        m_directoryPath = filePath.substr(0, dirEndPos);
    } else {
        m_directoryPath = ".";
    }
}

ProtobufReaderWriter::~ProtobufReaderWriter() {
    closeFile();
}

bool ProtobufReaderWriter::read(google::protobuf::Message &message) {
    openFileToRead();

    if (m_readFd == -1) {
        return false;
    }

    // Get file size by setting file offset to the end
    off_t fileSize;
    fileSize = ::lseek(m_readFd, 0, SEEK_END);
    if (fileSize == -1) {
        throw Exception("Could not seek the end of file: " + m_filePath);
    }
    // Reset file offset
    if (::lseek(m_readFd, 0, SEEK_SET) == -1) {
        throw Exception("Could not seek the start of file: " + m_filePath);
    }

    // Create a string to hold file contents
    std::string fileContent(fileSize, 0);

    // Read the file to string
    if (::read(m_readFd, &fileContent[0], fileSize) != fileSize) {
        throw Exception("I/O Error when reading file: " + m_filePath);
    }

    // Parse JSON string into protocol buffer message
    google::protobuf::util::Status status;
    google::protobuf::util::JsonParseOptions opts;

    // We want to have exact fields, when unknown one then we except error
    opts.ignore_unknown_fields = false;

    status = google::protobuf::util::JsonStringToMessage(fileContent, &message,
                                                         opts);

    return status.ok();
}

bool ProtobufReaderWriter::write(const google::protobuf::Message &message) {
    openFileToWrite();

    if (m_writeFd == -1) {
        if (m_errnoMsg != "") {
            log::cerr << "File could not be opened for writing: " << m_errnoMsg
                      << std::endl;
        }
        return false;
    }

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
    // Truncate file first
    if (::ftruncate(m_writeFd, 0) != 0) {
        return false;
    }

    int written = ::write(m_writeFd, str.data(), str.size());
    if (written < 0) {
        return false;
    }
    if (static_cast<unsigned>(written) != str.size()) {
        return false;
    }

    // 3. Flush write to disk
    if (::fsync(m_writeFd) != 0) {
        return false;
    }

    return true;
}

bool ProtobufReaderWriter::isFileAvailable() {
    // First we check if parent directory is available
    // If not - we throw an Exception as this is not expected
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

    if (m_readFd == -1) {
        return false;
    }

    // Check file type
    int result = ::fstat(m_readFd, &_stat);

    if (result == 0) {
        if (!S_ISREG(_stat.st_mode)) {
            // We expect regular file but it's not
            throw Exception("Expecting regular file, but it's not, " +
                            m_filePath);
        }
        return true;

    } else if (ENOENT == errno) {
        // File doesn't exist
        return false;

    } else {
        // Other error
        std::stringstream msg;
        msg << "Cannot access file: " << m_filePath << "ERROR(" << errno << ") "
            << strerror(errno);
        throw Exception(msg.str());
    }
}

bool ProtobufReaderWriter::remove() {
    // remove() will remove a symlink/hardlink and not what it points to,
    // so we don't have to worry about TOCTOU vulnerability.
    bool result = ::remove(m_filePath.c_str());
    if (result == 0) {
        closeFile();
        return true;
    }

    return false;
}

void ProtobufReaderWriter::openFileToRead() {
    if (m_readFd != -1) {
        // already opened
        return;
    }

    // File descriptor for reading
    m_readFd = ::open(m_filePath.c_str(), O_RDONLY | O_NOFOLLOW);

    if (m_readFd == -1) {
        // We should have at least read access - otherwise throw Exception
        if (errno == ELOOP) {
            throw Exception("Link files are not handled: " + m_filePath);
        }
    }
}

void ProtobufReaderWriter::openFileToWrite() {
    if (m_writeFd != -1) {
        // already opened
        return;
    }

    // File descriptor for writing
    m_writeFd = ::open(m_filePath.c_str(), O_WRONLY | O_CREAT | O_NOFOLLOW,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

    if (m_writeFd == -1) {
        // Opening for write failed - save error message, continue readonly
        m_errnoMsg = std::string(strerror(errno));

        if (errno == ELOOP) {
            throw Exception("Link files are not handled: " + m_filePath);
        }
    }
}

void ProtobufReaderWriter::closeFile() {
    if (m_readFd != -1) {
        if (::close(m_readFd) != 0) {
            throw Exception("Error when closing file: " +
                            std::string(strerror(errno)));
        }
        m_readFd = -1;
    }

    if (m_writeFd != -1) {
        if (::close(m_writeFd) != 0) {
            throw Exception("Error when closing file: " +
                            std::string(strerror(errno)));
        }
        m_writeFd = -1;
    }
}

bool ProtobufReaderWriter::makeReadOnly() {
    if (m_readFd == -1) {
        return false;
    }

    if (::fchmod(m_readFd, S_IRUSR | S_IRGRP) != 0) {
        return false;
    }

    // No write permissions now so we close descriptor for writing
    if (m_writeFd != -1) {
        ::close(m_writeFd);
        m_writeFd = -1;
    }

    return true;
}

}  // namespace octf
