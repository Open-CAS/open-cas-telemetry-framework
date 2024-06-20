add_library(protoexamples STATIC
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTest.proto
	${CMAKE_CURRENT_LIST_DIR}/test.proto
)
protobuf_generate(TARGET protoexamples
	IMPORT_DIRS ${OCTF_PROTOBUF_IMPORT_DIRS} ${CMAKE_CURRENT_LIST_DIR}
)
target_include_directories(protoexamples
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${OCTF_PROTO_DIR}
)
target_link_libraries(protoexamples PUBLIC protobuf::libprotobuf)

