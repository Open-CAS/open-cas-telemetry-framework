/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_COMMUNICATION_INTERNAL_ICLIENTEVENTHANDLER_H
#define SOURCE_OCTF_COMMUNICATION_INTERNAL_ICLIENTEVENTHANDLER_H

namespace octf {

/**
 * @interface IClientHandler
 *
 * This interface can be used by client side to get information about various
 * events. The example of events are:
 * - onAttach - client connected to server
 * - onDetach - client disconnected from server
 */
class IClientEventHandler {
public:
    IClientEventHandler() = default;
    virtual ~IClientEventHandler() = default;

    /**
     * @brief Client attached to the server
     *
     * Method is called whenever connection between server and client has
     * been established
     */
    virtual void onAttach() = 0;

    /**
     * Client detached form server
     *
     * Method is called whenever connection between server and client has
     * been ended
     */
    virtual void onDetach() = 0;
};

}  // namespace octf

#endif  // SOURCE_OCTF_COMMUNICATION_INTERNAL_ICLIENTEVENTHANDLER_H
