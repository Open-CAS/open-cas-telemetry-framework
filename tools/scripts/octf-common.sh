#!/bin/bash

# Copyright(c) 2012-2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause

OCTF_ROOT_DIR="$(cd $(dirname ${BASH_SOURCE[0]})/../../ && pwd)"

# Returns prefix for echoing line
function prefix () {
    local prefix="OCTF"
    if [ "${PREFIX}" != "" ]
    then
        prefix="${PREFIX}"
    fi

    echo ${prefix}
}

#
# Usage: check_result <RESULT> <ERROR_MESSAGE>
#
function check_result () {
    local result=$1
    local message=$2

    if [ ${result} -ne 0 ]
    then
        echo "[$(prefix)][ERROR] ${message}" 1>&2
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
    echo "[$(prefix)][INFO] $*"
}

function detect_distribution () {
    if [ -f /etc/redhat-release ]
    then
        if ( cat /etc/redhat-release | grep "Red Hat Enterprise Linux" | grep "release 7." &>/dev/null )
        then
            echo RHEL7
            return 0
        elif ( cat /etc/redhat-release | grep "Red Hat Enterprise Linux" | grep "release 8." &>/dev/null )
        then
            echo RHEL8
            return 0
        fi
    fi

    if [ -f /etc/centos-release ]
    then
        if ( cat /etc/centos-release | grep "CentOS Linux release 7." &>/dev/null )
        then
            echo CENTOS7
            return 0
        elif ( cat /etc/centos-release | grep "CentOS Linux release 8." &>/dev/null )
        then
            echo CENTOS8
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
        if ( cat /etc/os-release | grep "Debian" &>/dev/null )
        then
            echo DEBIAN
            return 0
        fi
    fi

    return 1
}

if [ "${DISTRO}" == "" ]
then
    export DISTRO=$(detect_distribution)
    case "${DISTRO}" in
    "RHEL7"|"RHEL8"|"CENTOS7"|"CENTOS8"|"FEDORA"|"UBUNTU"|"DEBIAN")
        info "${DISTRO} detected"
        ;;
    *)
        error "Unknown Linux distribution"
        exit 1
        ;;
    esac
fi

function get_distribution_pkg_manager () {
    case "${DISTRO}" in
    "RHEL7"|"RHEL8"|"CENTOS7"|"CENTOS8")
        echo "yum -y install"
        ;;
    "FEDORA")
        echo "dnf -y install"
        ;;
    "UBUNTU"|"DEBIAN")
        echo "apt-get -y install"
        ;;
    *)
        error "Unknown Linux distribution"
        ;;
    esac
}

#
# Usage: is_package_installed <PACKAGE>
#
function is_package_installed () {
    local cmd=""
    local pkg=$1

    if [ "${pkg}" == "" ]
    then
        error "No package specified to be checked if installed"
        exit 1
    fi

    case "${DISTRO}" in
    "RHEL7"|"RHEL8"|"CENTOS7"|"CENTOS8"|"FEDORA")
        cmd="rpm -q"
        ;;
    "UBUNTU"|"DEBIAN")
        cmd="dpkg -s"
        ;;
    *)
        error "Unknown Linux distribution"
        exit 1
        ;;
    esac

    ${cmd} ${pkg} &>/dev/null
    return $?
}

#
# Usage: get_distribution_missing_pkg_dependencies <PKG1> <PKG2> ...
#
function get_distribution_missing_pkg_dependencies () {
    local pkgs_required=$*
    local pkgs_missing=""

    for pkg in ${pkgs_required}
    do
        if ! is_package_installed ${pkg}
        then
            pkgs_missing="${pkg} ${pkgs_missing}"
        fi
    done

    echo ${pkgs_missing}
}

#
# Usage: install_pacakges PKG1 PKG2 ...
#
function install_pacakges () {
    local  pkgs=$*

    info "Required packages: ${pkgs}"
    pkgs=$(get_distribution_missing_pkg_dependencies $pkgs)

    local installer=$(get_distribution_pkg_manager)

    if [ "${pkgs}" != "" ]
    then
        info "Installing packages: ${pkgs}"
        ${installer} ${pkgs}
        check_result $? "Cannot install required dependencies"
    else
        info "All required packages already installed"
    fi
}
