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
        if ( cat /etc/fedora-release | grep "Fedora release 30" &>/dev/null )
        then
            echo FEDORA30
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

    if [ -f /etc/os-release ]
    then
        if ( cat /etc/os-release | grep "Ubuntu 16" &>/dev/null )
        then
            echo UBUNTU16
            return 0
        fi
    fi

    return 1
}

function setup_gtest
{
    if gtest_found
    then
        return
    fi

    info "Installing gtest to ${OPT_DIR}/gtest"

    local build_dir="$(dirname $0)/setup-dependencies-build"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    pushd "${build_dir}"

    git clone https://github.com/google/googletest
    check_result $? "Cannot clone Google Test"
    pushd googletest

    git checkout release-1.8.0
    check_result $? "Cannot checkout version GTest 1.8.0"

    mkdir -p build
    pushd build

    # CMake may be in opt
    export PATH="$PATH:${OPT_DIR}/cmake/bin/"

    cmake .. -DCMAKE_CXX_FLAGS=-fPIC && make -j$(nproc)
    check_result $? "Cannot compile GTest"

    mkdir -p ${OPT_DIR}/gtest/include
    cp -r ../googletest/include/* ${OPT_DIR}/gtest/include && \
    cp -r ./googlemock/gtest/libgtest*.a ${OPT_DIR}/gtest && \
    cp -r ./googlemock/libgmock*.a ${OPT_DIR}/gtest
    check_result $? "Cannot install gtest to ${OPT_DIR}"

    popd
    popd
    popd
    rm -rf "${build_dir}"

    info "Installed gtest to ${OPT_DIR}/gtest"
}

function setup_cmake
{
    if cmake_found
    then
        return
    fi

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

    mkdir -p ${OPT_DIR}/cmake

    tar -xzf ./cmake-${cmake_version}.tar.gz -C ${OPT_DIR}/cmake --strip-components=1
    check_result $? "Can't unpack CMake tarball"

    popd

    rm -rf "${build_dir}"
    info "Installed cmake to ${OPT_DIR}/cmake"
}

function gtest_found
{
    info "Looking for gtest in ${OPT_DIR}"
    if [ -d ${OPT_DIR}/gtest ]
    then
        info "Found Google Test  in ${OPT_DIR}"
        return 0
    fi

    info "No gtest found"
    return 1
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

    info "No cmake found"
    return 1;
}


if [ "$EUID" -ne 0 ]
then
    echo "Please run as root to alllow using apt/yum and installing to /opt"
    exit 1
fi

distro=$(detect_distribution)
case "${distro}" in
"RHEL7")
    info "RHEL7.x detected"
    packages="autoconf automake libtool curl make gcc-c++ unzip git"

    info "Installing packages: ${packages}"
    yum -y install ${packages}
    check_result $? "Cannot install required dependencies"

    setup_cmake
    setup_gtest
    ;;
"CENTOS7")
    info "CentOS7.x detected"
    packages="autoconf automake libtool curl make gcc-c++ unzip git"

    info "Installing packages: ${packages}"
    yum -y install ${packages}
    check_result $? "Cannot install required dependencies"

    setup_cmake
    setup_gtest
    ;;
"FEDORA30")
    info "Fedora 30 detected"
    packages="curl make gcc-c++ unzip cmake autoconf automake gtest-devel"

    info "Installing packages: ${packages}"
    dnf -y install ${packages}
    check_result $? "Cannot install required dependencies"
    ;;
"UBUNTU18")
    info "Ubuntu 18 detected"
    packages="cmake autoconf automake libtool curl make g++ unzip git"

    info "Installing packages: ${packages}"
    apt-get -y install ${packages}
    check_result $? "Cannot install required dependencies"

    setup_gtest
    ;;
"UBUNTU16")
    info "Ubuntu 16 detected"
    packages="autoconf automake libtool curl make g++ unzip git"

    info "Installing packages: ${packages}"
    apt-get -y install ${packages}
    check_result $? "Cannot install required dependencies"

    setup_cmake
    setup_gtest
    ;;
*)
    error "Unknown linux distribution"
    exit 1
    ;;
esac

