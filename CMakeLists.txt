cmake_minimum_required(VERSION 3.9)

include(octf-version.cmake)
project(octf VERSION ${OCTF_VERSION} LANGUAGES C CXX)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/third_party;${CMAKE_PREFIX_PATH}")

# Look in our directory for installed protobuf first

include(octf-paths.cmake)
include(octf-defenses.cmake)

add_subdirectory(source)
add_subdirectory(tests)
add_subdirectory(doc)
