add_subdirectory(parser)
target_sources(octf-tests PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/TracingTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TraceUtilsTest.h
)
