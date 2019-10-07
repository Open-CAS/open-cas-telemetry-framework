#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

MIN_PROTOBUF_VER_MAJOR=3
MIN_PROTOBUF_VER_MINOR=0

#TODO: check min proto version needed

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
    if [ -f /etc/redhat-release ] || [ -f /etc/centos-release ]
    then
        if ( cat /etc/redhat-release | grep "Red Hat Enterprise Linux Server release 7." &>/dev/null )
        then
            echo RHEL7
            return 0
        fi
    fi

    if [ -f /etc/redhat-release ] || [ -f /etc/centos-release ]
    then
        if ( cat /etc/centos-release | grep "CentOS Linux release 7." &>/dev/null )
        then
            echo CENTOS7
            return 0
        fi
    fi

    if [ -f /etc/os-release ]
    then
        if ( cat /etc/os-release | grep "Ubuntu 18" &>/dev/null )
        then
            echo UBUNTU18
            return 0
        fi
    fi

    return 1
}

function setup_protocol_buffer
{
    local build_dir="$(dirname $0)/octf-build"

    info "Setup google protocol buffer"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    pushd "${build_dir}"

    git clone https://github.com/protocolbuffers/protobuf.git
    check_result $? "Cannot clone google protocol buffer"
    pushd protobuf

    latest_release=$(curl --silent "https://github.com/protocolbuffers/protobuf/releases/latest" | sed 's#.*tag/\(.*\)\".*#\1#')
    if [ "${latest_release}" == "" ]
    then
        error "Cannot get latest version of google protocol buffer"
    fi

    git checkout ${latest_release}
    check_result $? "Cannot checkout version ${latest_release}"

    git submodule update --init --recursive && \
        ./autogen.sh && \
        ./configure --prefix=/usr && \
        make -j$(nproc) && \
        make check -j$(nproc) && \
        make install -j$(nproc) && \
        ldconfig
    check_result $? "Cannot setup google protocol buffer"

    popd
    popd

    rm -rf "${build_dir}"
}

function setup_rhel7 ()
{
    info "Install EPEL repository"
    yum -y install epel-release
    check_result $? "Cannot Install EPEL repository"

    info "Setup gtest"
    yum -y install gtest gtest-devel
    check_result $? "Cannot setup gtest"

    info "Setup cmake 3"
    yum -y install cmake3 && \
    alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake 10 \
        --slave /usr/local/bin/ctest ctest /usr/bin/ctest \
        --slave /usr/local/bin/cpack cpack /usr/bin/cpack \
        --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake \
        --family cmake \
    && \
    alternatives --install /usr/local/bin/cmake cmake /usr/bin/cmake3 20 \
        --slave /usr/local/bin/ctest ctest /usr/bin/ctest3 \
        --slave /usr/local/bin/cpack cpack /usr/bin/cpack3 \
        --slave /usr/local/bin/ccmake ccmake /usr/bin/ccmake3 \
        --family cmake

    check_result $? "Cannot setup alternative for cmake 3"

    setup_protocol_buffer
}

function setup_ubuntu18 ()
{
    local build_dir="$(dirname $0)/octf-build/gtest"

    info "Setup google test"

    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"

    pushd "${build_dir}"
    cmake /usr/src/gtest && \
    make -j$(nproc) && \
    make install -j$(nproc)
    check_result $? "Cannot setup google test"

    popd
    rm -rf "${build_dir}"
}

function protobuf_found ()
{
    info "Looking for protobuf"

    if command -v protoc >/dev/null
    then
        # Check existing protobuf version
        local protoc_version=$(protoc --version | awk {'print $2'})
        local protoc_version_major=$(printf "${protoc_version}" | awk -F '.' {'print $1'})
        local protoc_version_minor=$(printf "${protoc_version}" | awk -F '.' {'print $2'})

        if [ "$protoc_version_major" -lt "$MIN_PROTOBUF_VER_MAJOR" ]
        then
            info "Insufficent protobuf version found"
            return 1
        fi

        if [ "$protoc_version_minor" -lt "$MIN_PROTOBUF_VER_MINOR" ]
        then
            info "Insufficent protobuf version found"
            return 1
        fi

        info "Found protobuf, version: ${protoc_version}"
        return 0
    fi

    info "No protobuf found"
    return 1
}

function install_protobuf ()
{
    yes
}

protobuf_found
exit $?

distro=$(detect_distribution)
packages=""
installer=""
distro_setup=""

# check for protoc version
# if os is ok i versje to z paczki
# cmake tak samo
# gtest - wywalic test z all, nie ma fpica
# protobuf do opta
# slit - kernel header slub linux headers
# po instalacji sprawdzic uname'a czy /linux/srcs/kernels/$uname istnieje
# sprawdzic czy protobuf nie jest w opt/octf/protobuf

case "${distro}" in
"RHEL7")
    info "RHEL7.x/CentOS7.x detected"
    packages="cmake autoconf automake libtool curl make gcc-c++ unzip"
    installer="yum"
    distro_setup=setup_rhel7
    ;;
"CENTOS7")
    ;;
"UBUNTU18")
    info "Ubuntu 18 detected"
    packages="cmake autoconf automake libtool curl make g++ unzip
        libprotobuf-dev protobuf-compiler libgtest-dev"
    installer="apt-get"
    distro_setup=setup_ubuntu18
    ;;
*)
    error "Unknown linux distribution"
    exit 1
    ;;
esac

info "Install pacakges: ${packages}"
${installer} -y install ${packages}
check_result $? "Cannot install required dependencies"

${distro_setup}
