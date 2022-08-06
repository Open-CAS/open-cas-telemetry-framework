/*
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/trace/internal/TraceExtensionLocal.h>

#include <octf/interface/internal/FileTraceSerializer.h>
#include <octf/proto/traceDefinitions.pb.h>
#include <octf/trace/parser/TraceFileReader.h>
#include <octf/utils/Exception.h>
#include <octf/utils/FileOperations.h>
#include <octf/utils/FrameworkConfiguration.h>
#include <octf/utils/ProtobufReaderWriter.h>

namespace octf {

struct TraceExtensionLocal::Info {
    Info() = default;

    proto::TraceExtensionHeader hdr;
    std::string tracePath;
    std::string extName;
    std::string extPath;
    std::string lstPath;
};

class TraceExtensionLocal::Writer
        : public ITraceExtension::ITraceExtensionWriter {
public:
    Writer(TraceExtensionLocal::Info &info)
            : m_info(info)
            , m_sid()
            , m_lastSid(0)
            , m_serializer(info.extPath)
            , m_result(false) {
        if (!m_serializer.open()) {
            throw Exception("Extension ERROR, cannot open the serializer");
        }
    }
    virtual ~Writer() {}

    void write(uint64_t sid, const google::protobuf::Message &msg) override {
        if (sid < m_lastSid) {
            throw Exception("Extension ERROR, SID value invalid.");
        }

        // Serialize ID first
        m_sid.set_value(sid);
        if (!m_serializer.serialize(m_sid)) {
            throw Exception("Extension ERROR, cannot serialize SID");
        }

        // Now serialize the trace extension content
        if (!m_serializer.serialize(msg)) {
            throw Exception(
                    "Extension ERROR, cannot serialize the trace extension ");
        }

        m_lastSid = sid;
    }

    void commit() override {
        if (!m_serializer.close()) {
            throw Exception("Extension ERROR, cannot close the serializer");
        }

        ProtobufReaderWriter rw(m_info.lstPath);
        rw.lock();

        proto::TraceExtensionList lst;
        if (!rw.read(lst)) {
            throw Exception(
                    "Extension ERROR, Cannot get extention list in writer");
        }

        for (auto &extHdr : *lst.mutable_extension()) {
            if (extHdr.name() == m_info.extName) {
                extHdr.set_state(proto::TraceExtensionHeader::READY);
                m_result = rw.write(lst);
                break;
            }
        }

        if (!m_result) {
            throw Exception("Extension ERROR, Cannot commit the extention");
        } else {
            m_info.hdr.set_state(proto::TraceExtensionHeader::READY);
        }
    }

private:
    TraceExtensionLocal::Info &m_info;
    proto::Uint64 m_sid;
    uint64_t m_lastSid;
    FileTraceSerializer m_serializer;
    bool m_result;
};

class TraceExtensionLocal::Reader
        : public ITraceExtension::ITraceExtensionReader {
public:
    Reader(TraceExtensionLocal::Info &info)
            : ITraceExtension::ITraceExtensionReader()
            , m_info(info)
            , m_fileReader(info.extPath, 0)
            , m_sid()
            , m_hasSid(false) {
        m_fileReader.init();
    }
    virtual ~Reader() {}

    void read(uint64_t &sid, google::protobuf::Message &ext) override {
        // First get next SID
        sid = getNextSid();
        m_hasSid = false;

        // Then we can read the trace extension content
        m_fileReader.readTraceEvent(ext);
    }

    bool hasNext() override {
        return !m_fileReader.isFinished();
    }

    uint64_t getNextSid() {
        if (!m_hasSid) {
            m_fileReader.readTraceEvent(m_sid);
            m_hasSid = true;
        }

        return m_sid.value();
    }

private:
    TraceExtensionLocal::Info &m_info;
    TraceFileReader m_fileReader;
    proto::Uint64 m_sid;
    bool m_hasSid;
};

TraceExtensionLocal::TraceExtensionLocal(const std::string &tracePath,
                                         const std::string &extName)
        : ITraceExtension()
        , m_info(new TraceExtensionLocal::Info())
        , m_writer(nullptr)
        , m_reader(nullptr)
        , m_extrw(nullptr) {
    initTraceExtension(tracePath, extName);
}

TraceExtensionLocal::~TraceExtensionLocal() {
    if (isWritable()) {
        auto state = m_info->hdr.state();
        if (state == proto::TraceExtensionHeader::INITIALIZING ||
            state == proto::TraceExtensionHeader::ERROR) {
            // The trace extension not closed, an error ocurred, remove this
            // extension from the list
            this->remove();
        }
    }
}

const std::string &TraceExtensionLocal::getName() const {
    return m_info->hdr.name();
}

bool TraceExtensionLocal::isWritable() const {
    return nullptr != m_writer.get();
}

bool TraceExtensionLocal::isReady() const {
    return proto::TraceExtensionHeader::READY == m_info->hdr.state();
}

ITraceExtension::ITraceExtensionWriter &TraceExtensionLocal::getWriter() {
    if (!isWritable()) {
        throw Exception("ERROR, Extension is not writable, name " + getName());
    }

    return *m_writer;
}

ITraceExtension::ITraceExtensionReader &TraceExtensionLocal::getReader() {
    if (m_reader) {
        return *m_reader;
    } else {
        if (m_info->hdr.state() == proto::TraceExtensionHeader::READY) {
            // the extensions in god state, create Reader
            m_reader.reset(new Reader(*m_info));
            return *m_reader;
        }
    }

    throw Exception("ERROR, Extension is not ready, still writable, name " +
                    getName());
}

void TraceExtensionLocal::remove() {
    // First, remove trace extension from the list
    ProtobufReaderWriter rw(m_info->lstPath);
    rw.lock();

    // Second, remove trace extension file
    fsutils::removeFile(m_info->extPath);

    proto::TraceExtensionList oldLst, newLst;
    if (!rw.isEmpty()) {
        if (!rw.read(oldLst)) {
            throw Exception("ERROR, Cannot get extention list");
        }
    }

    // Third, build new extension list
    for (const auto &oldExtHdr : oldLst.extension()) {
        if (oldExtHdr.name() == m_info->extName) {
            continue;
        }

        auto newExtHdr = newLst.add_extension();
        *newExtHdr = oldExtHdr;
    }

    // Finally, write the new list
    if (!rw.write(newLst)) {
        throw Exception("Extension ERROR, Cannot remove the trace extension");
    }
}

static constexpr char TRACE_EXT_FILE_PREFIX[] = "octf.extension.";

void TraceExtensionLocal::initTraceExtension(const std::string &tracePath,
                                             const std::string &extName) {
    m_info->extName = extName;
    m_info->tracePath = tracePath;

    std::string pathPrefix = getFrameworkConfiguration().getTraceDir() + "/" +
                             tracePath + "/" + TRACE_EXT_FILE_PREFIX;

    m_info->lstPath = pathPrefix + "lst";
    ProtobufReaderWriter rw(m_info->lstPath);
    rw.lock();

    proto::TraceExtensionList lst;
    if (!rw.isEmpty()) {
        if (!rw.read(lst)) {
            throw Exception("ERROR, Cannot get extention list");
        }
    }

    uint64_t maxId = 0;

    // Iterate over available extensions and check if the extension exists.
    // In addition find out the next ID for the extension.
    proto::TraceExtensionHeader *extHdr, *found = NULL;
    for (int i = 0; i < lst.extension_size(); i++) {
        extHdr = lst.mutable_extension(i);

        if (extHdr->name() == m_info->extName) {
            if (found) {
                throw Exception("Extension ERROR, name duplication");
            }

            m_info->hdr = *extHdr;
            found = extHdr;
        }
        maxId = std::max(maxId, extHdr->id());
    }

    bool isWritable = false;
    if (m_info->hdr.name().empty()) {
        m_info->hdr.set_name(extName);
        m_info->hdr.set_id(++maxId);
        isWritable = true;

        // Set the extension path
        m_info->extPath = pathPrefix + std::to_string(m_info->hdr.id());
    } else {
        // Set the extension path
        m_info->extPath = pathPrefix + std::to_string(m_info->hdr.id());

        if (isStale()) {
            isWritable = true;
        }
    }

    if (isWritable) {
        // Update the extension header
        m_info->hdr.set_state(proto::TraceExtensionHeader::INITIALIZING);
        m_info->hdr.set_version(getFrameworkConfiguration().getVersion());

        if (!m_extrw) {
            m_extrw.reset(new ProtobufReaderWriter(m_info->extPath));
            m_extrw->lock();
        }

        if (!m_extrw->makeWritable() || !m_extrw->clear()) {
            throw Exception(
                    "ERROR, Cannot clear the trace extention for "
                    "refreshing");
        }

        if (found) {
            *found = m_info->hdr;
        } else {
            // Add new entry to extension list
            auto newExt = lst.add_extension();
            newExt->CopyFrom(m_info->hdr);
        }

        // Update the extension info
        if (!rw.write(lst)) {
            throw Exception("ERROR, Cannot update extention list");
        }

        // Create writer
        m_writer.reset(new Writer(*m_info));
    }
}

bool TraceExtensionLocal::isStale() {
    bool stale = false;

    if (isReady()) {
        if (m_info->hdr.version() != getFrameworkConfiguration().getVersion()) {
            // Version is different, regenerate the traces extension
            stale = true;
        }
    } else {
        // Let's try lock the extension, maybe it hadn't been finished.
        m_extrw.reset(new ProtobufReaderWriter(m_info->extPath));
        bool locked = m_extrw->tryLock();
        if (locked) {
            // The lock acquired, this indicates an error, previous writer
            // hadn't finished the job.
            stale = true;
        }
    }

    return stale;
}

}  // namespace octf
