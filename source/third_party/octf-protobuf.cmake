execute_process(
  COMMAND "cmake" "." "-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/protobuf" "-DCMAKE_CXX_FLAGS_INIT=-std=c++11 -fPIC" "-Dprotobuf_BUILD_TESTS=NO"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/cmake
)

execute_process(
  COMMAND "make" "install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/cmake
)
