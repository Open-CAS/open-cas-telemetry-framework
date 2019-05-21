add_subdirectory(unixsocket)
target_sources(octf-tests
PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/testDefines.h
)
