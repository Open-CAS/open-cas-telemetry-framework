#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

#
# This script validates and fixes if neccessary
# the C++ include guards in header files
# consisting of path to the file
#

VERBOSE=true

TRUE=0
FALSE=1


function main
{
    # Files that need include guard validation passed to this script
    FILES_TO_VALIDATE="$*"

    for filename in ${FILES_TO_VALIDATE}; do
        if [ -L "${filename}" ]; then
            continue;
        fi

        # If include guard has been found
        if is_include_guard_present; then

            # Fix if neccessary
            if ! is_include_guard_valid; then
                fix_include_guard

                if [ "${VERBOSE}" = true ]; then
                    echo "Fixed include guard in ${filename}"
                fi
            fi


        # Include guard has not been found - add it
        else
            add_include_guard
            if [ "${VERBOSE}" = true ]; then
                echo "Added include guard to ${filename}"
            fi
        fi

    done
}


function fix_include_guard
{
    # Find last endif line
    local endif_line=$( grep -n -i "#endif" "${filename}" | tac | cut -d : -f 1 | head -1)

    # Remove ifndef, define and endif
    sed -i -e "${ifndef_line}d;${define_line}d;${endif_line}d" "${filename}"

    # Add valid include guards
    add_include_guard
}


function is_include_guard_valid
{
    # Create proper value (PATH_FILENAME_H) for include guard
    # 1. Make upper case
    valid_guard_value=$(echo "${filename}" | awk  '{print toupper($0)}')

    # 2. Change all except [a-zA-Z0-9_] to '_'
    valid_guard_value=$(echo "${valid_guard_value}" | sed -E "s/[^a-zA-Z0-9_]/_/g")

    if [ "${found_guard_value}" == "${valid_guard_value}" ]; then
        return ${TRUE}
    else
        return ${FALSE}
    fi
}


# Check if file has include guards by looking for ifndef and define directives containing filename
function is_include_guard_present
{
    # Find first ifndef
    grep_result_ifndef=$(grep -i "#ifndef" "${filename}" | head -1)
    found_guard_value=$(echo "${grep_result_ifndef}" | cut -d " " -f 2)

    # Look for a define with the same value
    grep_result_define=$(grep -i "#define" "${filename}" | grep -i "${found_guard_value}")


    # Line numbers of found include guards - used for fixing if neccessary
    ifndef_line=$( grep -n -i "#ifndef" "${filename}" | grep -i "${found_guard_value}" | cut -d : -f 1 | head -1)
    define_line=$( grep -n -i "#define" "${filename}" | grep -i "${found_guard_value}" | cut -d : -f 1 | head -1)

    if [ -z "${grep_result_define}" ] || [ -z "${grep_result_ifndef}" ]; then
        # There is no include guard
        return ${FALSE}
    fi

    # Include guard found
    return ${TRUE}
}


# Add include guard to file without it
function add_include_guard
{
    # Create proper value (PATH_FILENAME_H) for include guard
    valid_guard_value=$(echo "${filename}" | awk  '{print toupper($0)}')
    valid_guard_value=$(echo "${valid_guard_value}" | sed -E "s/[^a-zA-Z0-9_]/_/g")

    # Find first line without comments so that include guard is after license header
    local guard_position=$(grep -E -v -x -n -m 1 "\s*((\*)|(\/\*)).*" "${filename}" | cut -d ':' -f 1)

    # Insert include guard in that line
    local guard="#ifndef ${valid_guard_value}\n#define ${valid_guard_value}\n"
    echo "$(head -n $((${guard_position} - 1)) ${filename})$(printf "\n\n${guard}\n")$(tail -n +${guard_position} ${filename})" > "${filename}"
    printf "\n#endif  // %s\n" "${valid_guard_value}" >> "${filename}"
}

main "$*"
