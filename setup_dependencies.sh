#!/bin/bash

# Copyright(c) 2012-2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

# This script dir name
OCTF_DIR="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"
. ${OCTF_DIR}/tools/scripts/octf-common.sh

function get_distribution_pkg_dependencies () {
    local pkgs_common="autoconf automake libtool curl make gcc unzip git wget \
                        doxygen make cmake"
    local pkgs_required=""

    case "${DISTRO}" in
    "RHEL7"|"RHEL8"|"CENTOS7"|"CENTOS8"|"FEDORA")
        pkgs_required="${pkgs_common} gcc-c++"
        ;;
    "UBUNTU"|"DEBIAN")
        pkgs_required="${pkgs_common} g++"
        ;;
    *)
        error "Unknown Linux distribution"
        exit 1
        ;;
    esac

    echo ${pkgs_required}
}

if [ "$EUID" -ne 0 ]
then
    error "Please run as root to allow using package manager"
fi

PKGS=$(get_distribution_pkg_dependencies)
install_pacakges ${PKGS}

script="${OCTF_DIR}/tools/scripts/octf-setup-cmake.sh"
${script}
if [ $? -ne 0 ]
then
    info "Cannot find or setup minimal version of cmake"
    info "Please provide it "
    info "Or execute ${script} when network access works"
    error "No minimal cmake version avaialbe"
fi

exit 0
