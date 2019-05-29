cmake_minimum_required(VERSION 3.9)

include(octf-version.cmake)
project(octf VERSION ${OCTF_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)

include(FindProtobuf)
find_package(Protobuf REQUIRED)

include(octf-paths.cmake)
include(octf-defenses.cmake)

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(doc)
