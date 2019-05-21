#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

#
# An example hook script to verify what is about to be committed.
# This script is called by "git commit" command. The hook should
# exit with non - zero status after issuing an appropriate message if
# it wants to stop the commit.
#
# This pre-commit hooks corrects coding style by executing
# clang-format with the style specified in .clang - format file.
# Script automatically adds modified versions of files.
#
# THIS FILE NEEDS TO BE COPIED OR LINKED AS TO APPEAR
# LOCALLY IN YOUR REPO AS: .git/hooks/pre-commit
#

# Files before modification are saved here
# WARNING: Previous content of this directory is deleted!
BACKUP_DIR="tools/coding_style/backup"
BACKUP_EXTENSION=".hbackup"

CHANGED_FILES=$(git diff --cached --name-only --diff-filter=AM)
CHANGED_FILES_CLANG=$(echo "${CHANGED_FILES}" | grep -Ee '\.([ch](pp|xx)|[h]|(cc|hh)|[CH]|proto)$')
CHANGED_FILES_LICENSE_HEADERS=$(echo "${CHANGED_FILES}" | grep -Ee '\.([chCH]|cc|hh|cpp|hpp|cxx|hxx|proto|sh|py)$')
CHANGED_FILES_INCLUDE_GUARDS=$(echo "${CHANGED_FILES}" |  grep -Ee '\.([hH]|hh|hpp|hxx)$')

function main {
    if [ -z "${CHANGED_FILES}" ]; then
        exit 0
    fi

    if ! [ -x "$(command -v clang-format)" ]; then
        echo "clang-format is not installed!"
        exit 1
    fi

    # Backup files
    backup_files

    if ! [ -z "${CHANGED_FILES_CLANG}}" ]; then
        # Apply coding style
        echo "${CHANGED_FILES_CLANG}" | xargs -i clang-format -style=file -i {}
    fi

    if ! [ -z  "${CHANGED_FILES_INCLUDE_GUARDS}" ]; then
        # Check and possibly fix include guards in header files
        ./tools/coding_style/include_guards.sh "${CHANGED_FILES_INCLUDE_GUARDS}"
    fi

    if ! [ -z "${CHANGED_FILES_LICENSE_HEADERS}" ]; then
        # Check and possibly fix license headers
        ./tools/coding_style/license_headers.sh "${CHANGED_FILES_LICENSE_HEADERS}"
    fi

    # Show changes using diff
    show_changes

    # Add changes
    echo "${CHANGED_FILES}" | xargs git add &> /dev/null
}

function show_changes {
    echo "Changes after executing pre-commit script:"

    while read -r file; do
        local diff_result=$(diff -y --suppress-common-lines "${file}" "${BACKUP_DIR}/${file}${BACKUP_EXTENSION}")

        # If there are differences
        if [ ! -z "${diff_result}" ]; then
            git --no-pager diff --no-index "${BACKUP_DIR}/${file}${BACKUP_EXTENSION}" "${file}"
        fi

    done <<< "${CHANGED_FILES}"
}

function backup_files {
    # Make sure backup dir exists
    mkdir -p "${BACKUP_DIR}"

    # Remove previous backups
    find "${BACKUP_DIR}" -type f -name "*${BACKUP_EXTENSION}" -exec rm -f {} \;

    # Remove empty directories
    find "${BACKUP_DIR}" -type d -empty -delete

     # For each file in files to backup
    while read -r file; do
        # Get file's directory
        local dir_name=$(dirname "${file}")

        # Recreate that directory in BACKUP_DIR
        mkdir -p "${BACKUP_DIR}/${dir_name}";

        # Copy file with added BACKUP_EXTENSION
        cp "${file}" "${BACKUP_DIR}/${file}${BACKUP_EXTENSION}"

    done <<< "${CHANGED_FILES}"
}

main
