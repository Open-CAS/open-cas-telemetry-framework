add_subdirectory(internal)
target_sources(octf
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/Executor.cpp
	${CMAKE_CURRENT_LIST_DIR}/Executor.h
	${CMAKE_CURRENT_LIST_DIR}/CLIProperties.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CLIProperties.h
)
