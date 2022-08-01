/*
 * Copyright(c) 2012-2022 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOURCE_OCTF_TRACE_ITRACEEXTENSION_H
#define SOURCE_OCTF_TRACE_ITRACEEXTENSION_H

#include <google/protobuf/message.h>
#include <memory>
#include <octf/utils/NonCopyable.h>

namespace octf {

/**
 * @brief This it the interface definition for extending trace by adding
 * extensions into the trace.
 *
 * User is able to create an extension for the trace and write additional
 * trace extensions. After a time user can read saved information.
 */
class ITraceExtension : public NonCopyable {
public:
    ITraceExtension() = default;
    virtual ~ITraceExtension() = default;

    /**
     * @brief Gets the Name of the trace extension
     *
     * @return The trace extension name
     */
    virtual const std::string &getName() const = 0;

    /**
     * @brief Checks if the trace extension is under construction and the writer
     * can be acquired
     *
     * @return true
     * @return false
     */
    virtual bool isWritable() const = 0;

    /**
     * @brief The interface of the trace extension writer
     */
    class ITraceExtensionWriter : public NonCopyable {
    public:
        ITraceExtensionWriter() = default;
        virtual ~ITraceExtensionWriter() = default;

        /**
         * @brief Writes an extension which reflects for the specific trace sid
         *
         * @param sid Trace sid for which extension is written
         * @param msg The extension trace massage to write
         */
        virtual void write(uint64_t sid,
                           const google::protobuf::Message &msg) = 0;

        /**
         * @brief Commit the trace extension
         *
         * It the writing procedure is finished the trace extension can be
         * committed by calling this function. After this, the reader can be
         * used for reading trace extensions.
         */
        virtual void commit() = 0;
    };

    /**
     * @brief Gets the writer for the extension
     *
     * @note Write can be used when the extension is under constriction.User can
     * call ITraceExtension::isWritable method to check if it's possible.
     *
     * @return Reference to the writer of this extension
     */
    virtual ITraceExtensionWriter &getWriter() = 0;

    /**
     * @brief When the extension
     *
     */
    class ITraceExtensionReader : public NonCopyable {
    public:
        ITraceExtensionReader() = default;
        virtual ~ITraceExtensionReader() = default;
        /**
         * @brief Reads the next trace extension
         *
         * @param[out] sid Trace sid which reflects to the read extension
         * @param[out] ext The extension to read
         */
        virtual void read(uint64_t &sid, google::protobuf::Message &ext) = 0;

        /**
         * @brief Check if reader has next trace extension to handle
         *
         * @retval true There is available the next trace extension
         * @return false No more trace extensions to read
         */
        virtual bool hasNext() = 0;

        /**
         * @brief Gets the sid of the next trace extension
         *
         * @return sid of the next trace extension
         */
        virtual uint64_t getNextSid() = 0;
    };

    virtual ITraceExtensionReader &getReader() = 0;

    /**
     * @brief Removes the trace extension
     */
    virtual void remove() = 0;
};

/**
 * @typedef Shared reference to a trace extension object
 */
typedef std::shared_ptr<ITraceExtension> TraceExtensionShRef;

}  // namespace octf

#endif  // SOURCE_OCTF_TRACE_ITRACEEXTENSION_H
