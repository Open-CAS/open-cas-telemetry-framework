set(protoListExamples
	InterfaceTest.proto
	test.proto
	)

protobuf_generate_cpp(PROTO_SRC PROTO_HEADER ${protoListExamples})
add_library(protoexamples STATIC ${PROTO_HEADER} ${PROTO_SRC} ${protoListExamples})
add_dependencies(protoexamples protooctf)
target_include_directories(protoexamples
	PUBLIC
	${PROTOBUF_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${OCTF_PROTO_DIR}
	)

