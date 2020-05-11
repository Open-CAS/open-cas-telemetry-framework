#!/bin/bash

# Copyright(c) 2012-2020 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

# This script dir name
OCTF_SCRIPT_DIR="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"
. ${OCTF_SCRIPT_DIR}/octf-common.sh

OCTF_CMAKE_MIN_VER_MAJOR=3
OCTF_CMAKE_MIN_VER_MINOR=10
OCTF_CMAKE_MIN_VER=${OCTF_CMAKE_MIN_VER_MAJOR}.${OCTF_CMAKE_MIN_VER_MINOR}

function cmake_info ()
{
    info "Minimal cmake version is ${OCTF_CMAKE_MIN_VER}"
}

function is_cmake_version_ok ()
{
    command -v cmake &>/dev/null
    if [ $? -ne 0 ]
    then
        return 1
    fi

    local prompt="cmake version "
    local version=$(cmake --version | grep "${prompt}" | sed -e s/^"${prompt}"//)

    if [ "${version}" == "" ]
    then
        echo b
        return 1
    fi

    local major=$(echo ${version} | awk -F '.' '{print $1}' )
    local minor=$(echo ${version} | awk -F '.' '{print $2}' )

    if [ "${major}" == "" ] || [ "${minor}" == "" ]
    then
        echo c
        return 1
    fi

    if [ ${major} -lt ${OCTF_CMAKE_MIN_VER_MAJOR} ]
    then
        return 1
    elif [ ${major} -eq ${OCTF_CMAKE_MIN_VER_MAJOR} ] && \
            [ ${minor} -lt ${OCTF_CMAKE_MIN_VER_MINOR} ]
    then
        return 1
    fi

    return 0
}

function setup_cmake
{
    cmake_info
    if is_cmake_version_ok
    then
        info "Installed cmake meets the minimum version requirements"
        return 0
    fi
    info "Cannot find appropriate cmake version"

    local cmake_dir=${OCTF_ROOT_DIR}/tools/third_party/cmake

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

setup_cmake
exit $?
