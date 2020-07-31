set(OCTF_VERSION_MAIN 20)
set(OCTF_VERSION_MAJOR 7)
set(OCTF_VERSION_MINOR 0)
set(OCTF_VERSION "${OCTF_VERSION_MAIN}.${OCTF_VERSION_MAJOR}.${OCTF_VERSION_MINOR}")

execute_process(
    COMMAND git log --pretty=format:%h -n 1
    OUTPUT_VARIABLE OCTF_VERSION_LABEL
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    ERROR_QUIET
)

message(STATUS "OCTF VERSION: ${OCTF_VERSION}")

if (NOT OCTF_VERSION_LABEL)
    message(STATUS "git repository not found, trying label from VERSION file")
    execute_process(
        COMMAND bash -c "grep LABEL < ${CMAKE_CURRENT_LIST_DIR}/VERSION | awk -F '=' '{print $2}' \
        | tr -d '\n'"
        OUTPUT_VARIABLE OCTF_VERSION_LABEL
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        ERROR_QUIET
   )
endif()

if (OCTF_VERSION_LABEL)
    message(STATUS "OCTF LABEL: ${OCTF_VERSION_LABEL}")
endif()

# Generate version file
file(
    WRITE ${CMAKE_CURRENT_BINARY_DIR}/VERSION
    "OCTF_VERSION_MAIN=${OCTF_VERSION_MAIN}\n"
)
file(
    APPEND ${CMAKE_CURRENT_BINARY_DIR}/VERSION
    "OCTF_VERSION_MAJOR=${OCTF_VERSION_MAJOR}\n"
)
file(
    APPEND ${CMAKE_CURRENT_BINARY_DIR}/VERSION
    "OCTF_VERSION_MINOR=${OCTF_VERSION_MINOR}\n"
)
if (OCTF_VERSION_LABEL)
    file(
        APPEND ${CMAKE_CURRENT_BINARY_DIR}/VERSION
        "OCTF_VERSION_LABEL=${OCTF_VERSION_LABEL}\n"
    )
endif()
