add_executable(testPluginClient "")
target_link_libraries(testPluginClient octf protoexamples)
target_sources(testPluginClient
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/TestPluginShadow.h
	${CMAKE_CURRENT_LIST_DIR}/TestPluginShadow.cpp
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
)
