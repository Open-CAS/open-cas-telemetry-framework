/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <octf/utils/table/Table.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <sstream>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Table::Table()
        : NonCopyable()
        , m_map(new TableMap()) {}

Table::~Table() {}

Row &Table::operator[](index_t row) {
    return m_map->getRow(row);
}

Row &Table::operator[](const std::string &row) {
    index_t index = m_map->getRowAssociation(row);
    return (*this)[index];
}

const Row &Table::operator[](index_t row) const {
    auto const &map = *m_map;

    return map.getRow(row);
}

const Row &Table::operator[](const std::string &row) const {
    index_t index = m_map->getRowAssociation(row);
    return (*this)[index];
}

RowIterator Table::begin() {
    return m_map->beginRow();
}

RowIterator Table::end() {
    return m_map->endRow();
}

RowIteratorConst Table::begin() const {
    auto const &map = *m_map;

    return map.beginRow();
}

RowIteratorConst Table::end() const {
    auto const &map = *m_map;

    return map.endRow();
}

void Table::clear() {
    m_map->clear();
}

index_t Table::size() const {
    auto const &map = *m_map;
    return map.size();
}

bool Table::empty() const {
    auto const &map = *m_map;
    return map.empty();
}

std::ostream &operator<<(std::ostream &os, const Table &table) {
    bool firstRow = true;

    for (const auto &row : table) {
        if (firstRow) {
            firstRow = false;
        } else {
            os << '\n';
        }

        bool firstColumn = true;
        for (const auto &cell : row) {
            if (firstColumn) {
                firstColumn = false;
            } else {
                os << ',';
            }

            os << cell;
        }
    }

    return os;
}

static void streamDescriptionToRow(Row &row,
                                   const ::google::protobuf::Descriptor *desc,
                                   const std::string &prefix) {
    using namespace google::protobuf;

    int count = desc->field_count();
    for (int i = 0; i < count; i++) {
        auto field = desc->field(i);

        if (field->is_repeated()) {
            continue;  // Not supported
        }

        string name = prefix + field->name();

        auto oneofDesc = field->containing_oneof();
        if (oneofDesc && oneofDesc->field_count()) {
            if (oneofDesc->field(0) == field) {
                string oneofName = prefix + oneofDesc->name();
                row[oneofName] = oneofName;
            }
        }

        if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            string nestedPrefix = name + ".";
            streamDescriptionToRow(row, field->message_type(), nestedPrefix);
        } else {
            row[name] = name;
        }
    }
}

Row &operator<<(Row &row, const ::google::protobuf::Descriptor *msg) {
    streamDescriptionToRow(row, msg, "");
    return row;
}

static void streamMessageToRow(Row &row,
                               const ::google::protobuf::Message &msg,
                               const std::string &prefix) {
    using namespace google::protobuf;

    auto reflection = msg.GetReflection();
    auto description = msg.GetDescriptor();

    int count = description->field_count();
    for (int i = 0; i < count; i++) {
        auto field = description->field(i);

        if (field->is_repeated() || field->is_map()) {
            continue;  // Not supported
        }

        std::string name = prefix + field->name();

        auto oneofDesc = field->containing_oneof();
        if (oneofDesc) {
            auto oneofField =
                    reflection->GetOneofFieldDescriptor(msg, oneofDesc);

            if (oneofField && oneofField == field) {
                std::string oneofName = prefix + oneofDesc->name();
                std::string oneofFieldName = prefix + oneofField->name();
                row[oneofName] = oneofFieldName;
            }
        }

        if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            if (reflection->HasField(msg, field)) {
                string nestedPrefix = name + ".";

                streamMessageToRow(row, reflection->GetMessage(msg, field),
                                   nestedPrefix);
            }
        } else {
            switch (field->cpp_type()) {
            case FieldDescriptor::CppType::CPPTYPE_INT32: {
                auto value = reflection->GetInt32(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_INT64: {
                auto value = reflection->GetInt64(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_UINT32: {
                auto value = reflection->GetUInt32(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_UINT64: {
                auto value = reflection->GetUInt64(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_DOUBLE: {
                auto value = reflection->GetDouble(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_FLOAT: {
                auto value = reflection->GetFloat(msg, field);
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_BOOL: {
                auto value = reflection->GetBool(msg, field);
                row[name] = value ? "true" : "false";
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_ENUM: {
                auto value = reflection->GetEnum(msg, field)->name();
                row[name] = value;
                break;
            }

            case FieldDescriptor::CppType::CPPTYPE_STRING: {
                const auto &value =
                        reflection->GetStringReference(msg, field, NULL);
                row[name] = value;
                break;
            }

            default:
                // Not supported
                break;
            }
        }
    }
}

Row &operator<<(Row &row, const ::google::protobuf::Message &msg) {
    streamMessageToRow(row, msg, "");
    return row;
}

}  // namespace table
}  // namespace octf
