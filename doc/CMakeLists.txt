cmake_minimum_required(VERSION 3.9)

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)
set(DOXYGEN_RECURSIVE YES)
set(DOXYGEN_EXCLUDE_PATTERNS "*/internal/*")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${PROJECT_SOURCE_DIR}/README.md")

find_package(Doxygen)

if(DOXYGEN_EXECUTABLE)
    doxygen_add_docs(
        doc
        ${PROJECT_SOURCE_DIR}/source/octf
        ${PROJECT_SOURCE_DIR}/README.md
        COMMENT "Generate doxygen documentation"
    )
else()
    add_custom_target(
        doc
        COMMAND false
        COMMENT "Cannot generate documentation because Doxygen is not installed"
    )
endif()
