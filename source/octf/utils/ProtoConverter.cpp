/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
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
bool contains(const google::protobuf::RepeatedPtrField<ElementType> array,
              ElementType value) {
    for (int i = 0; i < array.size(); i++) {
        if (array.Get(i) == value) {
            return true;
        }
    }

    return false;
}

google::protobuf::FileDescriptorSet
FileDescriptorSetCreator::createFileDescriptorSet() {
    google::protobuf::FileDescriptorSet fdSet;
    for (auto &fd : m_fds) {
        fdSet.add_file()->CopyFrom(fd);
    }

    return fdSet;
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
    FileDescriptorProto &fd = getFileDesc(msgDesc->file()->package());
    DescriptorProto *msgDescFd = fd.add_message_type();

    fd.set_name(msgDesc->file()->package());
    fd.set_package(msgDesc->file()->package());
    msgDesc->CopyTo(msgDescFd);

    // Describe field types of message or enum type
    for (int i = 0; i < msgDescFd->field_size(); i++) {
        const FieldDescriptor *field = msgDesc->field(i);

        // Handle message type
        if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            // Verify we only describe each type once
            if (std::find(m_knownTypes.begin(), m_knownTypes.end(),
                          field->full_name()) == m_knownTypes.end()) {
                // If we use a message type defined in different package, we
                // need to add appropriate import to FileDescriptor.
                std::string newTypePackage =
                        field->message_type()->file()->package();
                if (fd.package() != newTypePackage) {
                    // If not already imported
                    if (!contains<std::string>(fd.dependency(),
                                               newTypePackage)) {
                        fd.add_dependency(newTypePackage);
                    }
                }

                // Recursively add field's (of type message) definitions
                addMessageDesc(field->message_type());
                m_knownTypes.push_back(field->full_name());
            }
        }

        // Handle enum type
        if (field->type() == FieldDescriptor::Type::TYPE_ENUM) {
            // Verify we only describe each type once
            if (std::find(m_knownTypes.begin(), m_knownTypes.end(),
                          field->full_name()) == m_knownTypes.end()) {
                // If we use a enum type defined in different package, we
                // need to add appropriate import to FileDescriptor.
                std::string newTypePackage =
                        field->enum_type()->file()->package();
                if (fd.package() != newTypePackage) {
                    // If not already imported
                    if (!contains<std::string>(fd.dependency(),
                                               newTypePackage)) {
                        fd.add_dependency(newTypePackage);
                    }
                }

                // Define enum
                std::string package = field->enum_type()->file()->package();
                EnumDescriptorProto *enumDesc =
                        getFileDesc(package).add_enum_type();
                getFileDesc(package).set_package(package);
                getFileDesc(package).set_name(package);

                field->enum_type()->CopyTo(enumDesc);

                m_knownTypes.push_back(field->full_name());
            }
        }
    }
}

void FileDescriptorSetCreator::reset() {
    m_fds.clear();
    m_knownTypes.clear();
}

/**
 * This function looks for a file descriptor with given package in a list m_fds
 * If appropriate descriptor is not found, it is added. Reverse insertion order
 * is preserved to later allow parsing FileDescriptor one after another without
 * undefined types occurring.
 */
google::protobuf::FileDescriptorProto &FileDescriptorSetCreator::getFileDesc(
        std::string package) {
    for (auto &fd : m_fds) {
        if (fd.package() == package) {
            return fd;
        }
    }

    // FileDesc not found, create a new one
    google::protobuf::FileDescriptorProto newFd;
    newFd.set_name(package);
    newFd.set_package(package);

    m_fds.emplace_front(newFd);
    return *m_fds.begin();
}

}  // namespace protoconverter
}  // namespace octf
