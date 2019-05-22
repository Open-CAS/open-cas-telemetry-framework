#
# Build only selected files from safe string library into objects
#

set(SAFESTRINGLIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/memcmp_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/memset_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/mem_primitives_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/abort_handler_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/ignore_handler_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/safe_mem_constraint.c
)

add_library(safestringlib OBJECT ${SAFESTRINGLIB_SRC})

target_include_directories(safestringlib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/include
)
