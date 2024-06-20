add_executable(testPlugin "")
target_link_libraries(testPlugin octf protoexamples)
target_sources(testPlugin
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTestImpl.h
	${CMAKE_CURRENT_LIST_DIR}/TestPlugin.h
	${CMAKE_CURRENT_LIST_DIR}/InterfaceTestImpl.cpp
	${CMAKE_CURRENT_LIST_DIR}/main.cpp
	${CMAKE_CURRENT_LIST_DIR}/TestPlugin.cpp
)
