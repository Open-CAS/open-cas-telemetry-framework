set(WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/grpc")
set(PROTOC_OUT_DIR "${WORKING_DIRECTORY}/src/proto/grpc/reflection/v1alpha")

# Check if the file indicateing that the grpc library is ready is present
if(NOT EXISTS "${CMAKE_BINARY_DIR}/third_party/grpc.ready")
	include(ProcessorCount)
	ProcessorCount(N)

	execute_process(
		COMMAND "mkdir" "-p" "${WORKING_DIRECTORY}"
		RESULT_VARIABLE ret
	)

	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Creating directory for grpc library failed")
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND}
		"${CMAKE_CURRENT_SOURCE_DIR}/grpc"
		"-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/third_party"
		"-DCMAKE_CXX_FLAGS_INIT=${CMAKE_CXX_FLAGS_RELEASE}"
		"-DCMAKE_SHARED_LINKER_FLAGS_INIT=${CMAKE_SHARED_LINKER_FLAGS_RELEASE}"
		"-DCMAKE_EXE_LINKER_FLAGS_INIT=${CMAKE_EXE_LINKER_FLAGS_RELEASE}"
		"-DCMAKE_C_FLAGS_RELEASE_INIT=${CMAKE_C_FLAGS_RELEASE}"
		"-DCMAKE_BUILD_TYPE=Release"
		"-DABSL_ENABLE_INSTALL=ON"
		WORKING_DIRECTORY "${WORKING_DIRECTORY}"
		RESULT_VARIABLE ret
	)

	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "cmake for protobuf library failed")
	endif()

	execute_process(
		COMMAND "make" "install" "-j${N}"
		WORKING_DIRECTORY ${WORKING_DIRECTORY}
		RESULT_VARIABLE ret
	)

	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Building and installing of grpc library failed")
	endif()

	# Execute process to create an empty file ${CMAKE_BINARY_DIR}/third_party/grpc.ready
	execute_process(
		COMMAND "touch" "${CMAKE_BINARY_DIR}/third_party/grpc.ready"
		RESULT_VARIABLE ret
	)

	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Creating file ${CMAKE_BINARY_DIR}/third_party/grpc.ready failed")
	endif()

	execute_process(
		COMMAND "mkdir" "-p" "${PROTOC_OUT_DIR}"
		RESULT_VARIABLE ret
	)

	if(NOT ret EQUAL "0")
		message(FATAL_ERROR "Creating directory for generated protobuf files failed")
	endif()
endif()
