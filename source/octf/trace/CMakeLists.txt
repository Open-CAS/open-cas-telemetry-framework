add_subdirectory(parser)
add_subdirectory(internal)
target_sources(octf
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/trace.h
	${CMAKE_CURRENT_LIST_DIR}/trace.c
	${CMAKE_CURRENT_LIST_DIR}/iotrace_event.h
	${CMAKE_CURRENT_LIST_DIR}/IOTracePlugin.h
	${CMAKE_CURRENT_LIST_DIR}/IOTracePlugin.cpp
	${CMAKE_CURRENT_LIST_DIR}/ITrace.h
	${CMAKE_CURRENT_LIST_DIR}/ITraceRepository.h
	${CMAKE_CURRENT_LIST_DIR}/TraceLibrary.cpp
	${CMAKE_CURRENT_LIST_DIR}/TraceLibrary.h
	${CMAKE_CURRENT_LIST_DIR}/ITraceCache.h
	${CMAKE_CURRENT_LIST_DIR}/ITraceExtension.h
)
