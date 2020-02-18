add_executable(auto-cli "")
target_link_libraries(auto-cli octf ${PROTOBUF_LIBRARY})

target_include_directories(auto-cli
	SYSTEM PRIVATE ${PROTOBUF_INCLUDE_DIR}
)

# Generate protocol buffer headers and sources 
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER InterfaceHelloWorld.proto)

target_include_directories(auto-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(auto-cli PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_sources(auto-cli
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/InterfaceHelloWorldImpl.cpp
	${PROTO_SRC}
)
