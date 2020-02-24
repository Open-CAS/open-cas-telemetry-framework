#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

# This script dir name
OCTF_SCRIPT_DIR="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"

#
# Usage: check_result <RESULT> <ERROR_MESSAGE>
#
function check_result ()
{
    local result=$1
    local message=$2

    if [ ${result} -ne 0 ]
    then
        echo "[OCTF][ERROR] ${message}" 1>&2
        exit ${result}
    fi
}

#
# Usage: error <ERROR_MESSAGE_1> [ <ERROR_MESSAGE_2> ... ]
# Note: exits with error
#
function error () {
    check_result 255 "$*"
}

#
# Usage: info <INFO_MESSAGE_1> [ <INFO_MESSAGE_2> ... ]
#
function info () {
    echo "[OCTF][INFO] $*" 1>&2
}

function detect_distribution ()
{
    if [ -f /etc/redhat-release ]
    then
        if ( cat /etc/redhat-release | grep "Red Hat Enterprise Linux Server release 7." &>/dev/null )
        then
            echo RHEL7
            return 0
        fi
    fi

    if [ -f /etc/centos-release ]
    then
        if ( cat /etc/centos-release | grep "CentOS Linux release 7." &>/dev/null )
        then
            echo CENTOS7
            return 0
        fi
    fi

    if [ -f /etc/fedora-release ]
    then
        if ( cat /etc/fedora-release | grep "Fedora release" &>/dev/null )
        then
            echo FEDORA
            return 0
        fi
    fi

    if [ -f /etc/os-release ]
    then
        if ( cat /etc/os-release | grep "Ubuntu" &>/dev/null )
        then
            echo UBUNTU
            return 0
        fi
    fi

    return 1
}

function setup_cmake
{
    local cmake_dir=${OCTF_SCRIPT_DIR}/tools/third_party/cmake

    if [ ! -d "${cmake_dir}" ]
    then
        info "Create directory for cmake, ${cmake_dir}"
        mkdir -p ${cmake_dir}
        check_result $? "Can't create directory for cmake"
    fi

    local cmake_version="3.15.3"
    local cmake_arch="Linux-x86_64"
    local cmake_tar="cmake-${cmake_version}-${cmake_arch}.tar.gz"
    local cmake_tar_path="${cmake_dir}/${cmake_tar}"
    local cmake_link="https://github.com/Kitware/CMake/releases/download/v3.15.3/${cmake_tar}"
    local cmake_sha256="020812a9f87293482cec51fdf44f41cc47e794de568f945a8175549d997e1760"
    local cmake_executable="${cmake_dir}/bin/cmake"

    if [ ! -x "${cmake_executable}" ]
    then
        info "Setup cmake package"

        if [ -f "${cmake_tar_path}" ]
        then
             echo "${cmake_sha256} ${cmake_tar_path}" | sha256sum -c &>/dev/null
             if [ $? -ne 0 ]
             then
                info "CHECK sum is wrong: ${cmake_tar_path}"
                info "Remove package and re-download it"
                rm "${cmake_tar_path}"
             fi
        fi

        if [ ! -f "${cmake_tar_path}" ]
        then
            info "Download cmake package"
            wget -O ${cmake_tar_path} "${cmake_link}"
            check_result $? "Can't download CMake"
        fi

        echo "${cmake_sha256} ${cmake_tar_path}" | sha256sum -c
        check_result $? "Invalid SHA256 sum for downloaded file! Aborting."

        info "Extract cmake package"
        tar --no-same-owner -xzf ${cmake_tar_path} -C ${cmake_dir}/ --strip-components=1
        check_result $? "Can't unpack CMake tarball"

        if [ "${SUDO_USER}" != "" ]
        then
            info "Change owner of ${cmake_dir} to ${SUDO_USER}"
            chown -R ${SUDO_USER}:${SUDO_USER} ${cmake_dir}
            check_result $? "Can't change owner of ${cmake_dir} to ${SUDO_USER}"
        fi

        info "cmake installed to ${cmake_dir}"
    else
        info "cmake already installed to ${cmake_dir}"
    fi
}

function setup_dependencies () {
    setup_cmake
}

function get_distribution_pkg_manager () {
    distro=$(detect_distribution)
    case "${distro}" in
    "RHEL7"|"CENTOS7"|"FEDORA")
        echo "yum -y install"
        ;;
    "UBUNTU")
        echo "apt-get -y install"
        ;;
    *)
        error "Unknown Linux distribution"
        ;;
    esac
}

PKGS="autoconf automake libtool curl make gcc gcc-c++ unzip git wget"
function get_distribution_pkg_dependencies () {
    distro=$(detect_distribution)

    case "${distro}" in
    "RHEL7"|"CENTOS7"|"FEDORA")
        echo "${PKGS}"
        ;;
    "UBUNTU")
        echo "autoconf automake libtool curl make gcc g++ unzip git"
        ;;
    *)
        error "Unknown Linux distribution"
        exit 1
        ;;
    esac
}

if [ "$EUID" -ne 0 ]
then
    error "Please run as root to allow using package manager"
fi

setup_dependencies

DISTRO=$(detect_distribution)
if [ "" == "${DISTRO}" ]
then
    error "Cannot detect Linux distribution, please install packages yourself: ${PKGS}"
    exit 1
fi
info "${DISTRO} detected"

PKGS=$(get_distribution_pkg_dependencies)
PKG_INSTALLER=$(get_distribution_pkg_manager)

info "Installing packages: ${PKGS}"
${PKG_INSTALLER} ${PKGS}
check_result $? "Cannot install required dependencies"
