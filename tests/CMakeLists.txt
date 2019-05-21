cmake_minimum_required(VERSION 3.1)
project(test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

enable_testing()
find_package(GTest REQUIRED)
find_package (Threads)
add_executable(octf-tests "")

add_subdirectory(octf)

target_include_directories(octf-tests
	PUBLIC
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/source/
    ${CMAKE_BINARY_DIR}/source
)

target_link_libraries(octf-tests
    ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} gtest_main octf)

add_custom_target(run-unit-tests
    COMMAND octf-tests
)
