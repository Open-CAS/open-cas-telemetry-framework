#!/bin/bash

# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

# Minimal versions required
MIN_PROTOBUF_VER_MAJOR=3
MIN_PROTOBUF_VER_MINOR=0

MIN_CMAKE_VER_MAJOR=3
MIN_CMAKE_VER_MINOR=9

# Location for installing dependencies
OPT_DIR=/opt/octf


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

function setup_opt_dir
{
    if [ ! -d ${OPT_DIR} ]
    then
        mkdir -p ${OPT_DIR}
    fi
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

function setup_cmake
{
    info "Installing cmake to ${OPT_DIR}/cmake"

    local cmake_link="https://github.com/Kitware/CMake/releases/download/v3.15.3/cmake-3.15.3-Linux-x86_64.tar.gz"
    local cmake_sha256="020812a9f87293482cec51fdf44f41cc47e794de568f945a8175549d997e1760"
    local cmake_version="3.15.3-Linux-x86_64"

    local build_dir="$(dirname $0)/setup-dependencies-build"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    pushd "${build_dir}"

	wget "${cmake_link}"
    check_result $? "Can't download CMake"

	echo "${cmake_sha256}  ./cmake-${cmake_version}.tar.gz" | sha256sum -c
    check_result $? "Invalid SHA256 sum for downloaded file! Aborting."

    setup_opt_dir
    mkdir -p ${OPT_DIR}/cmake

    tar -xzf ./cmake-${cmake_version}.tar.gz -C ${OPT_DIR}/cmake --strip-components=1
    check_result $? "Can't unpack CMake tarball"

    popd

    rm -rf "${build_dir}"
}

function setup_protobuf
{
    info "Installing Google Protocol Buffers to ${OPT_DIR}/protobuf"

    local build_dir="$(dirname $0)/setup-dependencies-build"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    pushd "${build_dir}"

    git clone https://github.com/protocolbuffers/protobuf.git
    check_result $? "Cannot clone Google Protocol Buffers"
    pushd protobuf

    latest_release=$(curl --silent "https://github.com/protocolbuffers/protobuf/releases/latest" | sed 's#.*tag/\(.*\)\".*#\1#')
    if [ "${latest_release}" == "" ]
    then
        error "Cannot get the latest version of Google Protocol Buffers"
    fi

    git checkout ${latest_release}
    check_result $? "Cannot checkout version ${latest_release}"

    setup_opt_dir
    mkdir -p ${OPT_DIR}/protobuf

    git submodule update --init --recursive && \
        ./autogen.sh && \
        ./configure --prefix=${OPT_DIR}/protobuf && \
        make -j$(nproc) && \
        # TODO
        # make check -j$(nproc) && \
        make install -j$(nproc) && \
    check_result $? "Cannot setup Google Protocol Buffers"

    ldconfig
    check_result $? "Cannot run ldconfig after installing Google Protocol Buffers"

    popd
    popd

    rm -rf "${build_dir}"
}

function cmake_found
{
    info "Looking for cmake ${MIN_CMAKE_VER_MAJOR}.${MIN_CMAKE_VER_MINOR} or newer..."

    if [ -f ${OPT_DIR}/cmake/bin/cmake ]
    then
        local cmake_version=$(${OPT_DIR}/cmake/bin/cmake --version | awk /1/{'print $3'})
        info "Found cmake  in ${OPT_DIR}, version: ${cmake_version}"
        return 0
    fi

    if command -v cmake >/dev/null
    then
        # Check existing cmake version
        local cmake_version=$(cmake --version | awk /1/{'print $3'})
        local cmake_version_major=$(printf "${cmake_version}" | awk -F '.' {'print $1'})
        local cmake_version_minor=$(printf "${cmake_version}" | awk -F '.' {'print $2'})

        if [ "$cmake_version_major" -lt "$MIN_CMAKE_VER_MAJOR" ]
        then
            info "Insufficent cmake version found"
            return 1
        fi

        if [ "$cmake_version_minor" -lt "$MIN_CMAKE_VER_MINOR" ]
        then
            info "Insufficent cmake version found"
            return 1
        fi

        info "Found cmake, version: ${cmake_version}"
        return 0
    fi
}

function protobuf_found
{
    info "Looking for protobuf ${MIN_PROTOBUF_VER_MAJOR}.${MIN_PROTOBUF_VER_MINOR} or newer..."

    if [ -f ${OPT_DIR}/protobuf/bin/protoc ]
    then
        local protoc_version=$(${OPT_DIR}/protobuf/bin/protoc --version | awk {'print $2'})
        info "Found protobuf in ${OPT_DIR}, version: ${protoc_version}"
        return 0
    fi

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

if [ "$EUID" -ne 0 ]
then
    echo "Please run as root to alllow using apt/yum and installing to /opt"
    exit 1
fi

setup_protobuf
exit $?

distro=$(detect_distribution)
packages=""
installer=""
distro_setup=""

# if os is ok i versje to z paczki
# cmake tak samo
# gtest - wywalic test z all, nie ma fpica
# protobuf do opta
# slit - kernel header slub linux headers
# po instalacji sprawdzic uname'a czy /linux/srcs/kernels/$uname istnieje
# sprawdzic czy protobuf nie jest w opt/octf/protobuf

case "${distro}" in
"RHEL7")
    info "RHEL7.x detected"
    packages="cmake autoconf automake libtool curl make gcc-c++ unzip"
    installer="yum"
    setup_cmake
    setup_protobuf
    ;;
"CENTOS7")
    info "CentOS7.x detected"
    packages="cmake autoconf automake libtool curl make gcc-c++ unzip"
    installer="yum"
    setup_cmake
    setup_protobuf
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
