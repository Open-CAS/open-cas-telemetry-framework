include(ProcessorCount)
ProcessorCount(N)

set(WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
execute_process(
	COMMAND "mkdir" "-p" "${WORKING_DIRECTORY}"
	RESULT_VARIABLE ret
)

if(ret EQUAL "1")
	message(FATAL_ERROR "mkdir failed")
endif()

execute_process(
	COMMAND "cmake" "${CMAKE_CURRENT_SOURCE_DIR}/protobuf/cmake" "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third_party" "-DCMAKE_CXX_FLAGS_INIT=-std=c++11 -fPIC" "-Dprotobuf_BUILD_TESTS=NO"
	WORKING_DIRECTORY "${WORKING_DIRECTORY}"
	RESULT_VARIABLE ret
)

if(ret EQUAL "1")
	message(FATAL_ERROR "cmake failed")
endif()

execute_process(
	COMMAND "make" "install" "-j${N}"
	WORKING_DIRECTORY ${WORKING_DIRECTORY}
	RESULT_VARIABLE ret
)

if(ret EQUAL "1")
	message(FATAL_ERROR "make failed")
endif()
