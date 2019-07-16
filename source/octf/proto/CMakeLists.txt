set(protoList
	configuration.proto
	defs.proto
	InterfaceCLI.proto
	InterfaceIdentification.proto
	InterfaceTraceParsing.proto
	InterfaceService.proto
	InterfaceTraceCreating.proto
	InterfaceTraceManagement.proto
	InterfaceConfiguration.proto
	opts.proto
	packets.proto
	trace.proto
	parsedTrace.proto
	traceDefinitions.proto
	)
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${protoList})
add_library(protooctf OBJECT ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(protooctf
	PUBLIC
	${PROTOBUF_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	)
target_compile_options(protooctf PRIVATE -Wno-unused-parameter PUBLIC -fPIC)

# Set import dirs for protobuf and store them in 'global' variable OCTF_PROTOBUF_IMPORT_DIRS
# which can be used by projects utilizing octf's build system
set(PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}")
set(OCTF_PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} CACHE INTERNAL "OCTF_PROTOBUF_IMPORT_DIRS")

