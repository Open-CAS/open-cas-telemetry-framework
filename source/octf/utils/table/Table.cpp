/*
 * Copyright(c) 2012-2021 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <octf/utils/table/Table.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/map.h>
#include <google/protobuf/message.h>
#include <google/protobuf/reflection.h>
#include <sstream>
#include <octf/utils/Exception.h>
#include <octf/utils/Log.h>
#include <octf/utils/table/internal/TableMap.h>

namespace octf {
namespace table {

Table::Table()
        : m_map(new TableMap())
        , m_props()
        , m_title() {}

Table::~Table() {}

Table::Table(const Table &other)
        : IContainer<Row, RowIterator, RowIteratorConst>()
        , m_map(new TableMap(*other.m_map))
        , m_props(new Properties(*other.m_props))
        , m_title(other.m_title) {}

Table &Table::operator=(const Table &other) {
    if (this != &other) {
        m_map.reset(new TableMap(*other.m_map));
        m_props.reset(new Properties(*other.m_props));
        m_title = other.m_title;
    }

    return *this;
}

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

const octf::table::Properties &Table::getProperties() const {
    if (!m_props) {
        return Properties::builtin.csv();
    } else {
        return *m_props;
    }
}

Properties &Table::getProperties() {
    if (!m_props) {
        m_props.reset(new Properties(Properties::builtin.csv()));
    }

    return *m_props;
}

void Table::setProperties(const Properties &props) {
    if (!m_props) {
        m_props.reset(new Properties());
    }

    *m_props = props;
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

template <typename T>
static void streamRepeatedFieldToRow(
        Row &row,
        const ::google::protobuf::Message &msg,
        const ::google::protobuf::Reflection *reflection,
        const ::google::protobuf::FieldDescriptor *field,
        const std::string &name) {
    using namespace google::protobuf;

    auto repeated = reflection->GetRepeatedFieldRef<T>(msg, field);
    int i = 0;
    for (const auto &item : repeated) {
        std::string itemName = name + "[" + std::to_string(i) + "]";
        i++;

        row[itemName] = item;
    }
}

static void streamFieldToRow(Row &row,
                             const ::google::protobuf::Message &msg,
                             const ::google::protobuf::Reflection *reflection,
                             const ::google::protobuf::FieldDescriptor *field,
                             const std::string &name) {
    using namespace google::protobuf;

    bool isRepeated = field->is_repeated();

    switch (field->cpp_type()) {
    case FieldDescriptor::CppType::CPPTYPE_INT32: {
        if (isRepeated) {
            streamRepeatedFieldToRow<int32_t>(row, msg, reflection, field,
                                              name);
        } else {
            auto value = reflection->GetInt32(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_INT64: {
        if (isRepeated) {
            streamRepeatedFieldToRow<int64_t>(row, msg, reflection, field,
                                              name);
        } else {
            auto value = reflection->GetInt64(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_UINT32: {
        if (isRepeated) {
            streamRepeatedFieldToRow<uint32_t>(row, msg, reflection, field,
                                               name);
        } else {
            auto value = reflection->GetUInt32(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_UINT64: {
        if (isRepeated) {
            streamRepeatedFieldToRow<uint64_t>(row, msg, reflection, field,
                                               name);
        } else {
            auto value = reflection->GetUInt64(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_DOUBLE: {
        if (isRepeated) {
            streamRepeatedFieldToRow<double>(row, msg, reflection, field, name);
        } else {
            auto value = reflection->GetDouble(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_FLOAT: {
        if (isRepeated) {
            streamRepeatedFieldToRow<float>(row, msg, reflection, field, name);
        } else {
            auto value = reflection->GetFloat(msg, field);
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_BOOL: {
        if (isRepeated) {
            int size = reflection->FieldSize(msg, field);
            for (int i = 0; i < size; i++) {
                auto value = reflection->GetRepeatedBool(msg, field, i);
                std::string rName = name + "[" + std::to_string(i) + "]";
                row[rName] = value ? "true" : "false";
            }
        } else {
            auto value = reflection->GetBool(msg, field);
            row[name] = value ? "true" : "false";
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_ENUM: {
        if (isRepeated) {
            int size = reflection->FieldSize(msg, field);
            for (int i = 0; i < size; i++) {
                const auto &value =
                        reflection->GetRepeatedEnum(msg, field, i)->name();
                std::string rName = name + "[" + std::to_string(i) + "]";
                row[rName] = value;
            }
        } else {
            const auto &value = reflection->GetEnum(msg, field)->name();
            row[name] = value;
        }
        break;
    }

    case FieldDescriptor::CppType::CPPTYPE_STRING: {
        if (isRepeated) {
            streamRepeatedFieldToRow<std::string>(row, msg, reflection, field,
                                                  name);
        } else {
            const auto &value =
                    reflection->GetStringReference(msg, field, NULL);
            row[name] = value;
        }
        break;
    }

    default:
        // Not supported
        break;
    }
}

static void streamMessageToRow(Row &row,
                               const ::google::protobuf::Message &msg,
                               const std::string &prefix);

void static streamMapPairToRow(Row &row,
                               const ::google::protobuf::Message &pair,
                               const std::string &prefix) {
    using namespace google::protobuf;

    auto reflection = pair.GetReflection();
    auto description = pair.GetDescriptor();
    auto key = description->FindFieldByName("key");
    auto value = description->FindFieldByName("value");

    if (key == nullptr || value == nullptr) {
        throw Exception("Invalid map pair");
    }

    std::string name;

    switch (key->cpp_type()) {
    case FieldDescriptor::CppType::CPPTYPE_INT32: {
        auto value = reflection->GetInt32(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_INT64: {
        auto value = reflection->GetInt64(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_UINT32: {
        auto value = reflection->GetUInt32(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_UINT64: {
        auto value = reflection->GetUInt64(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_DOUBLE: {
        auto value = reflection->GetDouble(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_FLOAT: {
        auto value = reflection->GetFloat(pair, key);
        name = std::to_string(value);
    } break;

    case FieldDescriptor::CppType::CPPTYPE_BOOL: {
        auto value = reflection->GetBool(pair, key);
        name = value ? "true" : "false";
    } break;

    case FieldDescriptor::CppType::CPPTYPE_ENUM: {
        name = reflection->GetEnum(pair, key)->name();
    } break;

    case FieldDescriptor::CppType::CPPTYPE_STRING: {
        name = reflection->GetStringReference(pair, key, NULL);
    } break;

    default:
        // Not supported
        break;
    }

    name = prefix + "[" + name + "]";

    if (value->cpp_type() == FieldDescriptor::CppType::CPPTYPE_MESSAGE) {
        name += ".";
        streamMessageToRow(row, reflection->GetMessage(pair, value), name);
    } else {
        streamFieldToRow(row, pair, reflection, value, name);
    }
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

        std::string name = prefix + field->name();

        if (field->is_map()) {
            auto repeated =
                    reflection->GetRepeatedFieldRef<Message>(msg, field);
            for (const auto &item : repeated) {
                streamMapPairToRow(row, item, name);
            }
        } else if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            if (field->is_repeated()) {
                auto repeated =
                        reflection->GetRepeatedFieldRef<Message>(msg, field);

                int i = 0;
                for (const auto &item : repeated) {
                    std::string nestedPrefix;

                    nestedPrefix = name + "[" + std::to_string(i) + "].";
                    i++;
                    streamMessageToRow(row, item, nestedPrefix);
                }
            } else {
                if (!reflection->HasField(msg, field)) {
                    continue;
                }

                std::string nestedPrefix = name + ".";
                streamMessageToRow(row, reflection->GetMessage(msg, field),
                                   nestedPrefix);
            }
        } else {
            streamFieldToRow(row, msg, reflection, field, name);
        }
    }
}

Row &operator<<(Row &row, const ::google::protobuf::Message &msg) {
    streamMessageToRow(row, msg, "");
    return row;
}

static void setupRowAsHeader(Row &row,
                             const ::google::protobuf::Descriptor *desc,
                             const std::string &prefix) {
    using namespace google::protobuf;

    int count = desc->field_count();
    for (int i = 0; i < count; i++) {
        auto field = desc->field(i);

        if (field->is_repeated()) {
            // The repeated field description doesn't contain information about
            // its items. It has to be set using a message.
            continue;
        }

        std::string name = prefix + field->name();

        auto oneofDesc = field->containing_oneof();
        if (oneofDesc && oneofDesc->field_count()) {
            if (oneofDesc->field(0) == field) {
                std::string oneofName = prefix + oneofDesc->name();
                row[oneofName] = oneofName;
            }
        }

        if (field->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
            std::string nestedPrefix = name + ".";
            setupRowAsHeader(row, field->message_type(), nestedPrefix);
        } else {
            row[name] = name;
        }
    }
}

void setHeader(Row &row, const ::google::protobuf::Message *msg) {
    // First we store column association in temporary table
    TableMap map;

    // When setting header, we should name columns. We do two steps to
    // achieve this.
    //      First we use message content. It provides information about
    // primitive fields, allocated mutable messages, and especially it gives
    // reflection about dynamic objects like map, arrays. The information about
    // dynamic messages is not available in message descriptor. Thus we use
    // message itself.
    //      However message can have gaps in mutable messages (when fields not
    // set). Because of it we use descriptor to get info about all sub-messages
    // which are missing in the message.

    streamMessageToRow(map.getRow(0), *msg, "");
    setupRowAsHeader(map.getRow(0), msg->GetDescriptor(), "");

    // Re-write columns name from temporary table to the destination row
    auto iter = map.getColumnsAssociation().begin();
    auto end = map.getColumnsAssociation().end();
    for (; iter != end; iter++) {
        row[iter->first] = iter->first;
    }
}

}  // namespace table
}  // namespace octf
