/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <google/protobuf/descriptor.h>
#include <algorithm>
#include <list>
#include <octf/utils/ProtoConverter.h>

namespace octf {
namespace protoconverter {

void convertNodeId(proto::NodeId *nodeProto, const NodeId &node) {
    nodeProto->set_id(node.getId());
}

void convertInterfaceId(proto::InterfaceId *interfaceProto,
                        const InterfaceId &interface) {
    interfaceProto->set_name(interface.getName());
    interfaceProto->set_version(interface.getVersion());
}

void convertNodePath(proto::NodePath *nodePathProto, const NodePath &nodePath) {
    for (const auto &nodeId : nodePath) {
        convertNodeId(nodePathProto->add_node(), nodeId);
    }
}

int encodeVarint32(uint8_t *buffer, uint64_t size, int value) {
    unsigned int uValue = static_cast<unsigned int>(value);

    // Handle small values separately for performance
    if (uValue < 0x80) {
        if (size) {
            *buffer = uValue;
            size = 1;
            return 1;
        } else {
            return 0;
        }
    }

    uint32_t varIntSize = 0;

    do {
        if (0 == size) {
            return 0;
        }

        uint8_t byte = uValue & 0x7F;
        *buffer = byte;
        uValue >>= 7;

        if (uValue) {
            *buffer |= 0x80;
        }

        varIntSize++;
        buffer++;
        size--;

    } while (uValue);

    return varIntSize;
}

int decodeVarint32(const uint8_t *buffer, uint64_t size, int &value) {
    unsigned int uValue = 0;

    if (0 == size) {
        return 0;
    }

    // Handle small values separately for performance
    if (*buffer < 0x80) {
        value = *buffer;
        return 1;
    }

    for (int i = 0; i < MAX_VARINT32_BYTES; i++) {
        if (0 == size) {
            return 0;
        }

        uint8_t byte = *buffer;
        int byteNoMSB = byte & 0x7F;

        uValue |= byteNoMSB << (i * 7);

        if (byte < 0x80) {
            value = static_cast<int>(uValue);
            return i + 1;
        }

        buffer++;
        size--;
    }

    return 0;
}

template <class ElementType>
bool doesRepeatedPtrFieldContain(
        const google::protobuf::RepeatedPtrField<ElementType> &array,
        const ElementType &value) {
    for (int i = 0; i < array.size(); i++) {
        if (array.Get(i) == value) {
            return true;
        }
    }

    return false;
}

void FileDescriptorSetCreator::addMessageDesc(
        const google::protobuf::Descriptor *msgDesc) {
    using namespace google::protobuf;

    // We create a new definition in a proto file descriptor (runtime .proto
    // file equivalent) by copying a descriptor of the message we want to
    // describe as the new message type in file descriptor.
    // Any message's fields of 'message' or 'enum' type also need to be
    // described, as otherwise they will be undefined.
    //
    // To handle packages, we create a separate FileDescriptorProto for
    // each package. The name of the file is the package name.

    // Verify we only describe each message type once
    if (std::find(m_knownTypes.begin(), m_knownTypes.end(),
                  msgDesc->full_name()) != m_knownTypes.end()) {
        // Message already described
        return;
    }

    FileDescriptorProto *fd = getFileDesc(msgDesc->file()->package());
    DescriptorProto *msgDescFd = fd->add_message_type();

    msgDesc->CopyTo(msgDescFd);
    m_knownTypes.push_back(msgDesc->full_name());

    // Describe field types of message or enum type
    for (int i = 0; i < msgDescFd->field_size(); i++) {
        const FieldDescriptor *field = msgDesc->field(i);

        // Handle field of message type
        if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            // If we use a message type defined in different package, we
            // need to add appropriate import to FileDescriptor.
            const std::string &newTypePackage =
                    field->message_type()->file()->package();
            addDependency(fd, newTypePackage);

            // Recursively add field's (of type message) definitions
            addMessageDesc(field->message_type());

            // Handle field of enum type
        } else if (field->type() == FieldDescriptor::Type::TYPE_ENUM) {
            // Verify we only describe each enum type once
            if (std::find(m_knownTypes.begin(), m_knownTypes.end(),
                          field->full_name()) == m_knownTypes.end()) {
                const std::string &newTypePackage =
                        field->enum_type()->file()->package();
                // If we use an enum type defined in different package, we
                // need to add appropriate import to FileDescriptor.
                addDependency(fd, newTypePackage);

                // Define enum
                EnumDescriptorProto *enumDesc =
                        getFileDesc(newTypePackage)->add_enum_type();

                field->enum_type()->CopyTo(enumDesc);
                m_knownTypes.push_back(field->full_name());
            }
        }
    }
}

FileDescriptorSetCreator::FileDescriptorSetCreator(
        google::protobuf::FileDescriptorSet &fdSet)
        : m_knownTypes()
        , m_fdSet(fdSet) {}

/**
 * This function looks for a file descriptor with given package in m_fdSet
 * If appropriate descriptor is not found, it is added. Reverse insertion order
 * is preserved to later allow parsing FileDescriptor one after another without
 * undefined types occurring.
 */
google::protobuf::FileDescriptorProto *FileDescriptorSetCreator::getFileDesc(
        std::string package) {
    for (int i = 0; i < m_fdSet.file_size(); i++) {
        if (m_fdSet.file(i).package() == package) {
            return m_fdSet.mutable_file(i);
        }
    }

    // FileDesc not found, create a new one
    google::protobuf::FileDescriptorProto *newFd = m_fdSet.add_file();
    newFd->set_name(package);
    newFd->set_package(package);

    // Rearrange items afer inserting so that this element is added at the
    // beginning
    auto fds = m_fdSet.mutable_file();
    for (int i = m_fdSet.file_size() - 1; i > 0; i--) {
        fds->SwapElements(i, i - 1);
    }

    return newFd;
}

/**
 * Adds a 'package' dependency to 'fd'. Performs checks for existing dependency
 * and self-dependency
 */
void FileDescriptorSetCreator::addDependency(
        google::protobuf::FileDescriptorProto *fd,
        std::string package) {
    // Check if package is different than the current fd package
    if (fd->package() != package) {
        // If not already imported
        if (!doesRepeatedPtrFieldContain<std::string>(fd->dependency(),
                                                      package)) {
            fd->add_dependency(package);
        }
    }
}

}  // namespace protoconverter
}  // namespace octf
