/*
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <octf/utils/ProtobufReflection.h>

namespace octf {
namespace proto {

inline void Reflection::scan(const google::protobuf::Descriptor *desc,
                             IVisitorMessageDescriptor &visitor) {
    using namespace google::protobuf;

    int count = desc->field_count();
    for (int i = 0; i < count; i++) {
        auto fDesc = desc->field(i);
        pushPath(fDesc);

        switch (fDesc->type()) {
        case FieldDescriptor::Type::TYPE_MESSAGE:
            std::string prevPath = getPath();
            visitor.message(*this, fDesc, fDesc->message_type());
            scan(fDesc->message_type(), visitor);
            break;

        default:
            visitor.field(*this, fDesc);
            break;
        }

        popPath();
    }
}

inline const std::string &Reflection::getPath() const {
    if (m_path.empty()) {
        static const std::string empty;
        return empty;
    } else {
        return m_path.back();
    }
}

inline void Reflection::pushPath(
        const google::protobuf::FieldDescriptor *fDesc) {
    if (m_path.empty()) {
        m_path.push_back(fDesc->name());
    } else {
        std::string element = m_path.back() + ".";
        element += fDesc->name();
        m_path.push_back(element);
    }
}

inline void Reflection::popPath() {
    if (!m_path.empty()) {
        m_path.pop_back();
    }
}

}  // namespace proto
}  // namespace octf
