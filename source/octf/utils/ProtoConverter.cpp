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

google::protobuf::FileDescriptorSet
FileDescriptorSetCreator::createFileDescriptorSet() {
    google::protobuf::FileDescriptorSet fdSet;
    for (auto &fd : m_fds) {
        fdSet.add_file()->CopyFrom(fd.second);
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
    // each package. We keep them in a map indexed by package name.
    FileDescriptorProto &fd = m_fds[msgDesc->file()->package()];
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
                // Define enum
                EnumDescriptorProto *enumDesc =
                        m_fds[field->enum_type()->file()->package()]
                                .add_enum_type();
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

}  // namespace protoconverter
}  // namespace octf
