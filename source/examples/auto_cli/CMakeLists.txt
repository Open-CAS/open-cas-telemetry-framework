add_executable(auto-cli "")
target_link_libraries(auto-cli octf protobuf::libprotobuf)
target_include_directories(auto-cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(auto-cli PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_sources(auto-cli PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/InterfaceHelloWorldImpl.cpp
	${CMAKE_CURRENT_LIST_DIR}/InterfaceHelloWorld.proto
)

protobuf_generate(TARGET auto-cli
	LANGUAGE CPP
	IMPORT_DIRS ${OCTF_PROTOBUF_IMPORT_DIRS} ${CMAKE_CURRENT_LIST_DIR}
	GENERATE_EXTENSIONS .pb.h .pb.cc
)
