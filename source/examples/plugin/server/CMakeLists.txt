add_executable(testPlugin "")
target_link_libraries(testPlugin octf protoexamples)
target_link_libraries(testPlugin ${PROTOBUF_LIBRARIES})
target_include_directories(testPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(testPlugin PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/proto)
target_include_directories(testPlugin PRIVATE ${PROTOBUF_INCLUDE_DIR} ${OCTF_PROTO_DIR})
target_sources(testPlugin
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTestImpl.h
	${CMAKE_CURRENT_LIST_DIR}/TestPlugin.h
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTestImpl.cpp
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/TestPlugin.cpp
)
