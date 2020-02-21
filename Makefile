# Copyright(c) 2012-2018 Intel Corporation
# SPDX-License-Identifier: BSD-3-Clause-Clear

SHELL := /bin/bash
.DEFAULT_GOAL := all
CMAKE_FILE=CMakeLists.txt

TEST_DIR=build/test
TEST_PREFIX=./rootfs

ifdef DEBUG
	ifndef BUILD_DIR
		BUILD_DIR=build/debug
	endif
	BUILD_TYPE=DEBUG
	ifndef PREFIX
		PREFIX=./rootfs
	endif
else
	ifndef BUILD_DIR
		BUILD_DIR=build/release
	endif
	BUILD_TYPE=RELEASE
	ifndef PREFIX
		PREFIX=/
		RUN_LD_CONFIG=1
	endif
endif

SOURCE_PATH:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
CMAKE=$(SOURCE_PATH)/tools/third_party/cmake/bin/cmake
ifeq ("$(wildcard $(CMAKE))","")
   $(info Using system $(shell cmake --version | grep version))
   CMAKE=cmake
endif

.PHONY: init all install uninstall reinstall package clean doc

init:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) $(SOURCE_PATH) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_INSTALL_PREFIX=$(PREFIX)

all: init
	$(MAKE) -C $(BUILD_DIR) all

doc: init
	$(MAKE) -C $(BUILD_DIR) doc

install: all
	$(CMAKE)  -DCOMPONENT=octf-install -P $(BUILD_DIR)/cmake_install.cmake
	$(CMAKE)  -DCOMPONENT=octf-post-install -P $(BUILD_DIR)/cmake_install.cmake

uninstall: init
	$(MAKE) -C $(BUILD_DIR) octf-uninstall

reinstall: | uninstall install

package: all
	$(MAKE) -C $(BUILD_DIR) package

test:
	BUILD_DIR=$(TEST_DIR) PREFIX=$(TEST_PREFIX) $(MAKE) all
	BUILD_DIR=$(TEST_DIR) PREFIX=$(TEST_PREFIX) $(MAKE) install
	$(MAKE) -C $(TEST_DIR) run-unit-tests

clean:
	$(info Cleaning $(BUILD_DIR))
	@if [ -d $(BUILD_DIR) ] ; \
	then \
		$(MAKE) -C $(BUILD_DIR) clean ; \
		rm -rf $(BUILD_DIR) ; \
	fi
