#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

#
# This script checks files given as arguments for proper license header.
# If no header is found, the script prepends it
#

LICENSE_IDENTIFIER="SPDX-License-Identifier: BSD-3-Clause"

#
# usage: get_header <LINE COMMENT PREFIX>
#
# example: get_header "// "
# output: // SPDX-License-Identifier: BSD-3-Clause
#
function get_header ()
{
    echo ${LICENSE_IDENTIFIER} | sed "s/^/${1}/"
}

function get_header_c ()
{
    echo "/*"
    get_header "\ *\ "
    echo " */"
}

function get_header_hash ()
{
    get_header "# "
}

function is_header_present
{
    grep "${LICENSE_IDENTIFIER}" ${filename} &>/dev/null

    return $?
}

function insert_license
{
    declare -a lines

    IFS_BAK=$IFS
    IFS=$'\n'
    while read -r line
    do
        lines=( "$line"  "${lines[@]}")
    done <<< "$license"

    for line in "${lines[@]}"
    do
        if [ ! -s ${filename} ]
        then
            echo "$line" >> ${filename}
        else
            # line is passed into file by sed, so we need to escape
            # it in order to avoid conflict of sed special chars
            line=$(echo "$line" | sed -e 's/[]\/$*.^[]/\\&/g')

            # Check for shebang on the first two characters of file
            local first_line=$(head -1 ${filename})
            local initial_chars=${first_line:0:2}

            if [ "${initial_chars}" = "#!" ]
            then
                if [ $(echo $filename | wc -l) -eq 1 ]
                then
                    # It's one line file, so just add header line
                    # instead of inserting it using sed
                    echo "$line" >> ${filename}
                    continue
                fi

                # because of shebang insert header line after it
                i=2
            else
                i=1
            fi

            options="${i} s/^/${line}\n/"
            sed -i $options ${filename}
        fi
    done

    IFS=$IFS_BAK
    IFS_BAK=
}

function main
{
    # Get list of files passed as argument
    files=$*

    for filename in ${files}
    do
        if [ -L "${filename}" ]
        then
            continue
        fi

        # Determine needed license based on file extension and location in repository
        if [[ "${filename}" =~ .*\.([chCH]|cpp|hpp|cc|hh|proto)$ ]]
        then
            license="$(get_header_c)"
        elif [[ "${filename}" =~ .*\.(sh|py)$ ]]
        then
            # Script/text files
            license="$(get_header_hash)"
        else
            continue
        fi

        if is_header_present
        then
            continue
        fi

        echo "Fixing license header in file: ${filename}"
        insert_license
    done
}

main $*
