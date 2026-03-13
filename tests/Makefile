# CMake-backed compatibility Makefile for native and WebUI tests.

ROOT_DIR := $(abspath ..)
SRC_DIR := $(ROOT_DIR)/source
TEST_DIR := $(ROOT_DIR)/tests
THIRD_PARTY_DIR := $(ROOT_DIR)/third-party
BUILD_ROOT := $(ROOT_DIR)/build/cmake
DEFAULT_BUILD_DIR := $(BUILD_ROOT)/default
TEST_BUILD_DIR := $(ROOT_DIR)/tests/build
CMAKE ?= cmake

CATCH_INC := $(THIRD_PARTY_DIR)/Catch/single_include
THIRD_PARTY_C_FILES := $(THIRD_PARTY_DIR)/Tinyexpr/tinyexpr.c

TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp' -not -path '*/WebUI/*')
SRC_HPP := $(shell find $(SRC_DIR) -type f -name '*.hpp' -not -path '*/WebUI/*')
SRC_FILES := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp' -not -path '*/WebUI/*'),$(SRC_FILES))
OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(TEST_BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(TEST_BUILD_DIR)/source/%.o,$(SRC_FILES)) \
  $(patsubst $(THIRD_PARTY_DIR)/%,$(TEST_BUILD_DIR)/third-party/%.o,$(THIRD_PARTY_C_FILES))

.PHONY: all build test clean list web_test help

define configure_default
	$(CMAKE) -S $(ROOT_DIR) -B $(DEFAULT_BUILD_DIR) -DCOMPANYA_PROFILE=default
endef

all: test

build:
	$(call configure_default)
	$(CMAKE) --build $(DEFAULT_BUILD_DIR) --target run_tests

test: build
	ctest --test-dir $(DEFAULT_BUILD_DIR) --output-on-failure

web_test:
	$(call configure_default)
	$(CMAKE) --build $(DEFAULT_BUILD_DIR) --target web_test

list:
	@echo "Sources:"; echo "$(SRC_FILES)" | tr ' ' '\n'
	@echo
	@echo "Tests:"; echo "$(TEST_FILES)" | tr ' ' '\n'
	@echo
	@echo "Objects:"; echo "$(OBJ_FILES)" | tr ' ' '\n'

clean:
	$(CMAKE) -E rm -f $(TEST_BUILD_DIR)/run_tests
	$(CMAKE) -E rm -rf $(TEST_BUILD_DIR)/web_tests

help:
	@echo "Test targets:"
	@echo "  make / make test"
	@echo "  make build"
	@echo "  make web_test"
	@echo "  make list"
	@echo "  make clean"
