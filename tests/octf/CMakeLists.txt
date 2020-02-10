add_subdirectory(node)
add_subdirectory(socket)
add_subdirectory(utils)
add_subdirectory(trace)
add_subdirectory(analytics)
target_sources(octf-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/UtilsTest.h
)
