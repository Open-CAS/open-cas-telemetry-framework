# Path to OCTF sources and build directories
set(srcDir ${CMAKE_CURRENT_SOURCE_DIR}/octf)
set(buildDir ${CMAKE_CURRENT_BINARY_DIR}/octf)

add_library(octf SHARED ""
	 $<TARGET_OBJECTS:protooctf>
	 $<TARGET_OBJECTS:safestringlib>
)
set(OCTF_PROTO_DIR ${CMAKE_CURRENT_BINARY_DIR}/octf/proto)
add_subdirectory(octf)
add_subdirectory(octf/proto)
add_subdirectory(third_party)

set(PROTOBUF_IMPORT_DIRS  "${PROTOBUF_IMPORT_DIRS}" "${CMAKE_CURRENT_SOURCE_DIR}/octf/proto")

set_target_properties(octf PROPERTIES LINKER_LANGUAGE CXX)
target_compile_options(octf PRIVATE -Wall -Wextra)
target_include_directories(octf
	PUBLIC
	${PROTOBUF_INCLUDE_DIR}
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/octf/proto>
	$<INSTALL_INTERFACE:${OCTF_INCLUDE_DIR}/octf/proto>
	$<INSTALL_INTERFACE:${OCTF_INCLUDE_DIR}/octf>
)
# Specify libraries to link
target_link_libraries(octf PUBLIC pthread)
target_link_libraries(octf PUBLIC ${PROTOBUF_LIBRARY})

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

# Install octf.so library and add octf target to export group 'octf-targets'
install(TARGETS octf DESTINATION ${OCTF_LIBRARY_DIR} EXPORT octf-targets COMPONENT octf-install)

if (${CMAKE_INSTALL_PREFIX} STREQUAL "/")
    install(
        CODE "execute_process(COMMAND ldconfig)"
        COMPONENT octf-install
    )
endif()


# Install headers and proto files
install(DIRECTORY octf ${CMAKE_CURRENT_BINARY_DIR}/octf
	DESTINATION ${OCTF_INCLUDE_DIR}
	COMPONENT octf-install
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.proto"
	REGEX "internal" EXCLUDE
	REGEX "CMakeFiles" EXCLUDE)

# Generate and install a .cmake file with exported targets and properties definitions
install(EXPORT octf-targets DESTINATION ${OCTF_PACKAGE_DIR} FILE octf.cmake COMPONENT octf-install)

# Install a file which will be recognized by 'find_package' and includes above file with exports
install(FILES ${CMAKE_CURRENT_LIST_DIR}/../octf-config.cmake DESTINATION ${OCTF_PACKAGE_DIR} COMPONENT octf-install)

# Create a file with variables which are to be exported with the package
file(WRITE ${CMAKE_BINARY_DIR}/octf-vars.cmake
	"set(OCTF_PROTOBUF_IMPORT_DIRS \"${OCTF_INCLUDE_DIR}/octf/proto\")\n"
)
file(APPEND ${CMAKE_BINARY_DIR}/octf-vars.cmake
	"set(OCTF_INCLUDE_DIRS \"${OCTF_INCLUDE_DIR}/octf;${OCTF_INCLUDE_DIR}/octf/proto\")\n"
)

# Install it next to octf-config
install(FILES ${CMAKE_BINARY_DIR}/octf-vars.cmake DESTINATION ${OCTF_PACKAGE_DIR} COMPONENT octf-install)

add_subdirectory(examples)
