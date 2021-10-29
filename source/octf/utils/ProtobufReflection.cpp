/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <octf/utils/ProtobufReflection.h>

#include <octf/utils/Exception.h>

namespace octf {
namespace proto {

Reflection::Reflection()
        : m_path()
        , m_level(0) {}

void Reflection::scan(const google::protobuf::Descriptor *desc,
                      IVisitorMessageDescriptor &visitor) {
    using namespace google::protobuf;

    int count = desc->field_count();
    for (int i = 0; i < count; i++) {
        auto fDesc = desc->field(i);
        pushPath(fDesc);

        switch (fDesc->type()) {
        case FieldDescriptor::Type::TYPE_MESSAGE:
            fieldMessage(fDesc, visitor);
            break;

        default:
            visitor.field(*this, fDesc);
            break;
        }

        popPath();
    }
}

const std::string &Reflection::getPath() const {
    if (m_path.empty()) {
        static const std::string empty;
        return empty;
    } else {
        return m_path.back();
    }
}

void Reflection::pushPath(const google::protobuf::FieldDescriptor *fDesc) {
    if (m_path.empty()) {
        m_path.push_back(fDesc->name());
    } else {
        std::string element = m_path.back() + "." + fDesc->name();

        if (fDesc->is_repeated()) {
            element += "[index]";
        }
        if (fDesc->is_map()) {
            element += "[key]";
        }

        m_path.push_back(element);
    }
}

void Reflection::popPath() {
    if (!m_path.empty()) {
        m_path.pop_back();
    }
}

bool Reflection::pushMessage(const google::protobuf::Descriptor *msg) {
    auto iter = std::find(m_msgs.begin(), m_msgs.end(), msg);
    if (iter != m_msgs.end()) {
        return false;
    }

    m_level++;
    m_msgs.push_back(msg);
    return true;
}

void Reflection::popMessage() {
    if (!m_msgs.empty()) {
        m_level--;
        m_msgs.pop_back();
    } else {
        throw Exception("Message scanning ERROR, nested message missing");
    }
}

void Reflection::fieldMessage(const google::protobuf::FieldDescriptor *fDesc,
                              IVisitorMessageDescriptor &visitor) {
    const auto mDesc = fDesc->message_type();
    bool pushed = pushMessage(mDesc);

    visitor.message(*this, fDesc, mDesc);

    if (pushed) {
        scan(mDesc, visitor);
        popMessage();
    }
}

}  // namespace proto
}  // namespace octf
