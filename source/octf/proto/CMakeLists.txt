add_library(protooctf OBJECT
	${CMAKE_CURRENT_LIST_DIR}/configuration.proto
	${CMAKE_CURRENT_LIST_DIR}/defs.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceCLI.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceIdentification.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceParsing.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceService.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceCreating.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTraceManagement.proto
	${CMAKE_CURRENT_LIST_DIR}/InterfaceConfiguration.proto
	${CMAKE_CURRENT_LIST_DIR}/opts.proto
	${CMAKE_CURRENT_LIST_DIR}/packets.proto
	${CMAKE_CURRENT_LIST_DIR}/trace.proto
	${CMAKE_CURRENT_LIST_DIR}/parsedTrace.proto
	${CMAKE_CURRENT_LIST_DIR}/traceDefinitions.proto
	${CMAKE_CURRENT_LIST_DIR}/statistics.proto
	${CMAKE_CURRENT_LIST_DIR}/extensions.proto
)
target_link_libraries(protooctf PUBLIC protobuf::libprotobuf)
target_compile_options(protooctf PRIVATE -Wno-unused-parameter PUBLIC -fPIC)

# Set import dirs for protobuf and store them in 'global' variable OCTF_PROTOBUF_IMPORT_DIRS
# which can be used by projects utilizing octf's build system
set(PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}")
set(OCTF_PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} CACHE INTERNAL "OCTF_PROTOBUF_IMPORT_DIRS")

protobuf_generate(TARGET protooctf)
