add_executable(example-cli "")
target_link_libraries(example-cli octf)

# Generate protocol buffer headers and sources 
protobuf_generate_cpp(PROTO_SRC PROTO_HEADER InterfaceHelloWorld.proto)

target_include_directories(example-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(example-cli PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_sources(example-cli
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/InterfaceHelloWorldImpl.cpp
	${PROTO_SRC}
)
