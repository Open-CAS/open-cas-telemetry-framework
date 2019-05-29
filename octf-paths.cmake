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

    # If custom installation path is set,
    # prepend it to default paths
    if(CMAKE_INSTALL_PREFIX STREQUAL "" OR CMAKE_INSTALL_PREFIX STREQUAL "/")
        set(${dir} ${${dir}})
    else()
        set(${dir} ${CMAKE_INSTALL_PREFIX}/${${dir}})
    endif()
    # Remove double backslashes etc.
    get_filename_component(${dir} ${${dir}} REALPATH)
endforeach(dir)

set(configFileName "octf.conf")

set(NOTICE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/doc/NOTICE")
set(LICENSE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/VERSION")
set(OCTF_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/${configFileName})

set(socketsPath ${OCTF_RUN_DIR}/sockets)
set(tracePath ${OCTF_WORK_DIR}/trace)
set(settingsPath ${OCTF_WORK_DIR}/settings)

set(destConfigFile "${OCTF_CONFIG_DIR}/${configFileName}")
add_definitions(-DOCTF_CONFIG_FILE="${destConfigFile}")
message(STATUS "OCTF_CONFIG_FILE set to: ${destConfigFile}")

set(includePath ${OCTF_INCLUDE_DIR})

file(WRITE ${OCTF_CONFIG_FILE}
    "{\n"
       "   \"paths\": {\n"
       "   \"settings\": \"${settingsPath}\",\n"
       "   \"unixsocket\": \"${socketsPath}\",\n"
       "   \"trace\": \"${tracePath}\",\n"
       "   }\n"
    "}")

install(FILES ${OCTF_CONFIG_FILE}
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
        COMPONENT octf-install
       )

install(DIRECTORY
        DESTINATION ${settingsPath}
        COMPONENT octf-install
       )
