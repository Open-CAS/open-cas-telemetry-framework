#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear


# This script creates ${outputFileName} file with includes to files given
# in semicolon separated file list. Relative paths are created by removing
# everything up to and including 'frameworkSA' from file path
# The header file is then saved to to specified path
#
# This script should be called by cmake

outputFileName="octf.h"
headersToInclude="${1}"
outputDir="${2}"
outputHeaderContent="/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef OCTF_H
#define OCTF_H"

IFS=";"
for i in ${headersToInclude}
do
    # Turn path into relative path (remove everything up to 'octf')
    relative=$(echo "${i}" | sed 's/^.*octf/octf/')

    # Create #include
    outputHeaderContent+=$(echo -e "
    #include <${relative}>")
done

outputHeaderContent+=$(echo "
#endif // OCTF_H")

echo "${outputHeaderContent}" > "${outputDir}/${outputFileName}"
