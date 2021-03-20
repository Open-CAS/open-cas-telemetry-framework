include("${CMAKE_CURRENT_LIST_DIR}/tools/installer/installation_config.cmake")

set(DIRS
    OCTF_INCLUDE_DIR;
    OCTF_LIBRARY_DIR;
    OCTF_BIN_DIR;
    OCTF_CONFIG_DIR;
    OCTF_RUN_DIR;
    OCTF_WORK_DIR;
    OCTF_DOC_DIR;
    OCTF_PACKAGE_DIR)

foreach(dir ${DIRS})
    if(NOT ${dir})
        message(FATAL_ERROR ${dir} " not defined!")
    endif()

    # Remove double backslashes etc.
    get_filename_component(${dir} ${${dir}} REALPATH)
    set(${dir} ${${dir}} CACHE INTERNAL ${dir})
endforeach(dir)

set(OCTF_CONFIG_FILE_NAME "octf.conf"  CACHE INTERNAL OCTF_CONFIG_FILE_NAME)

set(NOTICE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/NOTICE")
set(LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/VERSION")
set(OCTF_CONFIG_FILE_PROJECT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${OCTF_CONFIG_FILE_NAME} CACHE INTERNAL OCTF_CONFIG_FILE_PROJECT_PATH)
set(OCTF_MANIFEST_PREINSTALL ${CMAKE_BINARY_DIR}/install_manifest_octf-install.txt)
set(OCTF_MANIFEST_POSTINSTALL ${OCTF_DOC_DIR}/install_manifest_octf-install.txt)

set(runPath ${OCTF_RUN_DIR})
set(socketsPath ${OCTF_RUN_DIR}/sockets)
set(tracePath ${OCTF_WORK_DIR}/trace)
set(settingsPath ${OCTF_WORK_DIR}/settings)

set(OCTF_CONFIG_FILE_PATH "${OCTF_CONFIG_DIR}/${OCTF_CONFIG_FILE_NAME}" CACHE INTERNAL OCTF_CONFIG_FILE_PATH)
add_definitions(-DOCTF_CONFIG_FILE="${OCTF_CONFIG_FILE_PATH}")
message(STATUS "OCTF_CONFIG_FILE set to: ${OCTF_CONFIG_FILE_PATH}")

set(includePath ${OCTF_INCLUDE_DIR})
file(WRITE ${OCTF_CONFIG_FILE_PROJECT_PATH}
    "{\n"
       "   \"paths\": {\n"
       "   \"settings\": \"${settingsPath}\",\n"
       "   \"unixsocket\": \"${socketsPath}\",\n"
       "   \"trace\": \"${tracePath}\",\n"
       "   \"run\": \"${runPath}\"\n"
       "   }\n"
    "}")

install(FILES ${OCTF_CONFIG_FILE_PROJECT_PATH}
        DESTINATION ${OCTF_CONFIG_DIR}
        COMPONENT octf-install
       )

install(FILES ${VERSION_FILE}
        DESTINATION ${OCTF_DOC_DIR}
        COMPONENT octf-install
       )

install(FILES ${NOTICE_FILE}
        DESTINATION ${OCTF_DOC_DIR}
        COMPONENT octf-install
       )

install(FILES ${LICENSE_FILE}
        DESTINATION ${OCTF_DOC_DIR}
        COMPONENT octf-install
       )

install(DIRECTORY
        DESTINATION ${socketsPath}
        COMPONENT octf-install
       )

install(DIRECTORY
        DESTINATION ${tracePath}
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE
        COMPONENT octf-install
       )

install(DIRECTORY
        DESTINATION ${runPath}
        COMPONENT octf-install
       )
