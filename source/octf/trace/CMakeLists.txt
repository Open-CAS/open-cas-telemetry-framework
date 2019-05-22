add_subdirectory(parser)
add_subdirectory(internal)
target_sources(octf
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/trace.h
	${CMAKE_CURRENT_LIST_DIR}/trace.c
	${CMAKE_CURRENT_LIST_DIR}/iotrace_event.h
)
