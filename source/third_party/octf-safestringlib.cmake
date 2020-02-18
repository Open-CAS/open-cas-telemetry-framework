include(CheckIncludeFile)

#
# Build only selected files from safe string library into objects
#

set(SAFESTRINGLIB_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/safe_str_constraint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/strcpy_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/memcpy_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/memset_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/mem_primitives_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/abort_handler_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/ignore_handler_s.c
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/safeclib/safe_mem_constraint.c
)

add_library(safestringlib OBJECT ${SAFESTRINGLIB_SRC})

CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
if(HAVE_STDLIB_H)
    target_compile_definitions(
        safestringlib
        PRIVATE
        HAVE_STDLIB_H
    )
endif()

CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
if(HAVE_MEMORY_H)
    target_compile_definitions(
        safestringlib
        PRIVATE
        HAVE_MEMORY_H
    )
endif()

CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
if(HAVE_STRING_H)
    target_compile_definitions(
        safestringlib
        PRIVATE
        HAVE_STRING_H
    )
endif()

CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS_H)
if(HAVE_LIMITS_H)
    target_compile_definitions(
        safestringlib
        PRIVATE
        HAVE_LIMITS_H
    )
endif()

target_include_directories(safestringlib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/safestringlib/include
)

target_compile_options(safestringlib PRIVATE -fpic)
