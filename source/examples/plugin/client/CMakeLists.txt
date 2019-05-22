add_executable(testPluginClient "")
target_include_directories(testPluginClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(testPluginClient PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/proto)
target_include_directories(testPluginClient PRIVATE ${PROTOBUF_INCLUDE_DIR})
target_sources(testPluginClient
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/TestPluginShadow.h
	${CMAKE_CURRENT_LIST_DIR}/TestPluginShadow.cpp
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
)
target_link_libraries(testPluginClient octf)
target_link_libraries(testPluginClient protoexamples)
target_link_libraries(testPluginClient ${PROTOBUF_LIBRARIES})
