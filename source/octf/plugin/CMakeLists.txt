add_subdirectory(internal)
target_sources(octf
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/NodePluginShadow.cpp
	${CMAKE_CURRENT_LIST_DIR}/NodePluginShadow.h
	${CMAKE_CURRENT_LIST_DIR}/NodePlugin.h
	${CMAKE_CURRENT_LIST_DIR}/NodePlugin.cpp
)
