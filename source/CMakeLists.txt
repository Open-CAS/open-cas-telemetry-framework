# Path to OCTF sources and build directories
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR}/octf)
set(buildDir ${CMAKE_CURRENT_BINARY_DIR}/octf)

add_subdirectory(third_party)
find_package(Protobuf CONFIG REQUIRED)

add_library(octf SHARED ""
	$<TARGET_OBJECTS:protooctf>
	$<TARGET_OBJECTS:safestringlib>
)
set_target_properties(octf PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(octf PRIVATE -Wall -Wextra)

set(OCTF_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/octf/proto)

target_include_directories(octf
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/octf/proto>
	$<INSTALL_INTERFACE:${OCTF_INCLUDE_DIR}/octf/proto>
	$<INSTALL_INTERFACE:${OCTF_INCLUDE_DIR}/octf>
)

# Specify libraries to link
target_link_libraries(octf PUBLIC pthread)
target_link_libraries(octf PUBLIC protobuf::libprotobuf)
target_link_libraries(octf PRIVATE fort)

add_subdirectory(octf)

# Add version definitions
target_compile_definitions(octf
	PUBLIC
	OCTF_VERSION=${OCTF_VERSION}
	OCTF_VERSION_LABEL=${OCTF_VERSION_LABEL}
)

# Define static version of octf library and copy all properties from shared one
add_library(octf-static STATIC EXCLUDE_FROM_ALL ""
	$<TARGET_OBJECTS:protooctf>
	$<TARGET_OBJECTS:safestringlib>
)

get_target_property(OCTF_SRCS octf SOURCES)
get_target_property(OCTF_LIBS octf LINK_LIBRARIES)
get_target_property(OCTF_LINKER octf LINKER_LANGUAGE)
get_target_property(OCTF_INC octf INCLUDE_DIRECTORIES)
get_target_property(OCTF_OPTS octf COMPILE_OPTIONS)
get_target_property(OCTF_DEFS octf COMPILE_DEFINITIONS)

set_property(TARGET octf-static PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(octf-static PRIVATE "${OCTF_SRCS}")
target_link_libraries(octf-static "${OCTF_LIBS}")
target_link_libraries(octf-static fort)

set_target_properties(octf-static PROPERTIES LINKER_LANGUAGE "${OCTF_LINKER}")
set_target_properties(octf-static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OCTF_INC}")
set_target_properties(octf-static PROPERTIES INCLUDE_DIRECTORIES "${OCTF_INC}")
set_target_properties(octf-static PROPERTIES COMPILE_OPTIONS "${OCTF_OPTS}")
set_target_properties(octf-static PROPERTIES COMPILE_DEFINITIONS "${OCTF_DEFS}")

file(GLOB_RECURSE protoFiles
	${srcDir}/proto/*.proto)
file(GLOB_RECURSE publicHeaders
	${srcDir}/*.h)

# Replace all .proto extensions to pb.h, this is needed in order to
# get a list of pb.h files before they are generated
string(REPLACE "\.proto" "\.pb.h" protoHeaders "${protoFiles}")

# Modify path from source to build
STRING(REGEX REPLACE
	"${srcDir}([^;]*\.pb\.h)"
	"${buildDir}\\1"
	protoHeaders
	"${protoHeaders}")
set(publicHeaders ${publicHeaders} ${protoHeaders})

# Remove headers not added to sdk (from internal directories)
FOREACH(item ${publicHeaders})
	IF(${item} MATCHES ".*internal\/.*")
		LIST(REMOVE_ITEM publicHeaders ${item})
	ENDIF(${item} MATCHES ".*internal\/.*")
ENDFOREACH(item)

# Generate and copy octf.h header file, do this in pre-link phase
add_custom_command(
	TARGET octf
	PRE_LINK
	COMMAND ${srcDir}/generateCommonHeader.sh "\"${publicHeaders}\"" ${buildDir}
)
add_custom_command(
	TARGET octf-static
	PRE_LINK
	COMMAND ${srcDir}/generateCommonHeader.sh "\"${publicHeaders}\"" ${buildDir}
)

# Install install-manifest with list of installed files, to allow uninstalling
# Append install-manifest to itself
install(CODE
	"file(APPEND ${OCTF_MANIFEST_PREINSTALL} \"\n${OCTF_MANIFEST_POSTINSTALL}\")"
	COMPONENT octf-post-install
)
install(FILES ${OCTF_MANIFEST_PREINSTALL}
	DESTINATION ${OCTF_DOC_DIR}
	COMPONENT octf-post-install
)

# Add a target for uninstallation. It's best done on the CMakeLists.txt level,
# because we must use GNUUtils to obtain the proper path to install-manifest
add_custom_target(octf-uninstall
	COMMAND test -f ${OCTF_MANIFEST_POSTINSTALL} &&
	xargs rm -vf < ${OCTF_MANIFEST_POSTINSTALL} ||
	echo "-- No OCTF install manifest found: ${OCTF_MANIFEST_POSTINSTALL} Nothing to uninstall!"
)

# Install octf.so library and add octf target to export group 'octf-targets'
install(TARGETS octf DESTINATION ${OCTF_LIBRARY_DIR} EXPORT octf-targets COMPONENT octf-install)

if(${CMAKE_INSTALL_PREFIX} STREQUAL "/")
	install(
		CODE "execute_process(COMMAND ldconfig)"
		COMPONENT octf-post-install
	)
endif()

# Install headers and proto files
install(DIRECTORY octf ${CMAKE_CURRENT_BINARY_DIR}/octf
	DESTINATION ${OCTF_INCLUDE_DIR}
	COMPONENT octf-install-headers
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.proto"
	REGEX "internal" EXCLUDE
	REGEX "CMakeFiles" EXCLUDE)

# Generate and install a .cmake file with exported targets and properties definitions
install(EXPORT octf-targets DESTINATION ${OCTF_PACKAGE_DIR} FILE octf.cmake COMPONENT octf-install-cmake)

# Install a file which will be recognized by 'find_package' and includes above file with exports
install(FILES ${CMAKE_CURRENT_LIST_DIR}/../octf-config.cmake DESTINATION ${OCTF_PACKAGE_DIR} COMPONENT octf-install-cmake)

# Create a file with variables which are to be exported with the package
file(WRITE ${CMAKE_BINARY_DIR}/octf-vars.cmake
	"set(OCTF_PROTOBUF_IMPORT_DIRS \"${OCTF_INCLUDE_DIR}/octf/proto\")\n"
)
file(APPEND ${CMAKE_BINARY_DIR}/octf-vars.cmake
	"set(OCTF_INCLUDE_DIRS \"${OCTF_INCLUDE_DIR}/octf;${OCTF_INCLUDE_DIR}/octf/proto\")\n"
)

# Install it next to octf-config
install(FILES ${CMAKE_BINARY_DIR}/octf-vars.cmake DESTINATION ${OCTF_PACKAGE_DIR} COMPONENT octf-install-cmake)

add_subdirectory(examples)
