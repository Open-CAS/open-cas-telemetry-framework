add_subdirectory(v0)
add_subdirectory(v1)
add_subdirectory(v2)
add_subdirectory(extensions)

target_sources(octf
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/IoTraceEventHandlerCsvPrinter.h
    ${CMAKE_CURRENT_LIST_DIR}/IoTraceEventHandlerJsonPrinter.h
    ${CMAKE_CURRENT_LIST_DIR}/ITraceParser.h
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandler.h
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandlerPrinter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandlerPrinter.h
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandlerStatistics.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandler.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerCsvPrinter.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerJsonPrinter.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerWorkset.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerWorkset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerDevicesList.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerDevicesList.cpp
	${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandlerExtensionBuilder.h
	${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventHandlerExtensionBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TraceFileParser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TraceFileParser.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceFileReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TraceFileReader.h
    ${CMAKE_CURRENT_LIST_DIR}/HandlerRunner.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerFilesystemStatistics.h
    ${CMAKE_CURRENT_LIST_DIR}/TraceEventHandlerFilesystemStatistics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventQueue.h
    ${CMAKE_CURRENT_LIST_DIR}/ParsedIoTraceEventQueue.cpp
)
