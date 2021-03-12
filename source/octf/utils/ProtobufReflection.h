/*
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_PROTOBUFREFLECTION_H
#define SOURCE_OCTF_UTILS_PROTOBUFREFLECTION_H
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <list>
#include <string>
#include <octf/utils/NonCopyable.h>

namespace octf {
namespace proto {

class Reflection;

/**
 * @interface IVisitorMessageDescriptor
 *
 * Visitor interface used by Reflection
 */
class IVisitorMessageDescriptor {
public:
    IVisitorMessageDescriptor() = default;
    virtual ~IVisitorMessageDescriptor() = default;

    /**
     * @brief Informs about the field in the protocol buffer message
     *
     * @param reflection Reflection which performs scanning on the protocol
     * buffer message/descriptor
     * @param fDesc Reported field descriptor
     */
    virtual void field(const Reflection &reflection,
                       const google::protobuf::FieldDescriptor *fDesc) = 0;

    /**
     * @brief Informs about the nested message in the protocol buffer message
     *
     * @param reflection Reflection which performs scanning on the protocol
     * buffer message/descriptor
     * @param fDesc Reported field descriptor
     * @param mDesc Reported message descriptor
     */
    virtual void message(const Reflection &reflection,
                         const google::protobuf::FieldDescriptor *fDesc,
                         const google::protobuf::Descriptor *mDesc) = 0;
};

/**
 * @brief Utility class which helps scan protocol buffer message and descriptor
 * recursively
 */
class Reflection : public NonCopyable {
public:
    Reflection() = default;
    virtual ~Reflection() = default;

    /**
     * @brief Scans protocol buffer message descriptor
     *
     * @param desc Protocol buffer message descriptor to be scanned
     * @param visitor Visitor which wants to scan the descriptor
     */
    void scan(const google::protobuf::Descriptor *desc,
              IVisitorMessageDescriptor &visitor);

    /**
     * @brief Gets the path of the element which is currently scanned
     * @return The path of the element
     */
    const std::string &getPath() const;

private:
    void pushPath(const google::protobuf::FieldDescriptor *fDesc);
    void popPath();

private:
    std::list<std::string> m_path;
};

}  // namespace proto
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_PROTOBUFREFLECTION_H
