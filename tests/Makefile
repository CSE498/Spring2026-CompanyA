# Makefile for Catch2 unit tests
#
# Assumptions:
# - Production sources live under: source/
# - Tests live under: tests/ (and mirror source/ directory structure)
#
# Usage:
#   make            # build + run tests
#   make test       # run tests
#   make build      # build test binary only
#   make clean

SRC_DIR   := ../source
TEST_DIR  := ../tests
THIRD_PARTY_DIR := ../third-party
BUILD_DIR := ../tests/build/
BIN       := $(BUILD_DIR)/run_tests

CATCH_INC := $(THIRD_PARTY_DIR)/Catch/single_include

CXXFLAGS ?= -O2 -g
CFLAGS   ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC)
LDFLAGS  ?=
LDLIBS   ?=

# Pick up .cpp from tests/
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp')

# Pick up headers from source/, then compile only matching .cpp next to them
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp'),$(SRC_FILES))

# Pick up headers from third-party/, then compile only matching implementation
# files next to them. Exclude Catch because tests already provide main via
# single-header mode (CATCH_CONFIG_MAIN).
THIRD_PARTY_HPP      := $(shell find $(THIRD_PARTY_DIR) -type f -name '*.hpp')
THIRD_PARTY_H        := $(shell find $(THIRD_PARTY_DIR) -type f -name '*.h')
THIRD_PARTY_CPP      := $(patsubst %.hpp,%.cpp,$(THIRD_PARTY_HPP)) \
                        $(patsubst %.h,%.cpp,$(THIRD_PARTY_H))
THIRD_PARTY_C        := $(patsubst %.h,%.c,$(THIRD_PARTY_H))
THIRD_PARTY_CPP_FILES := $(filter-out $(THIRD_PARTY_DIR)/Catch/%,\
                          $(filter $(shell find $(THIRD_PARTY_DIR) -type f -name '*.cpp'),$(THIRD_PARTY_CPP)))
THIRD_PARTY_C_FILES   := $(filter-out $(THIRD_PARTY_DIR)/Catch/%,\
                          $(filter $(shell find $(THIRD_PARTY_DIR) -type f -name '*.c'),$(THIRD_PARTY_C)))
THIRD_PARTY_FILES     := $(THIRD_PARTY_CPP_FILES) $(THIRD_PARTY_C_FILES)

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES)) \
  $(patsubst $(THIRD_PARTY_DIR)/%.cpp,$(BUILD_DIR)/third-party/%.cpp.o,$(THIRD_PARTY_CPP_FILES)) \
  $(patsubst $(THIRD_PARTY_DIR)/%.c,$(BUILD_DIR)/third-party/%.c.o,$(THIRD_PARTY_C_FILES))

DEP_FILES := $(OBJ_FILES:.o=.d)

.PHONY: all build test clean list

all: test

build: $(BIN)

test: $(BIN)
	$(BIN)

# Link test runner
$(BIN): $(OBJ_FILES) | $(BUILD_DIR)
	$(CXX) -std=c++23 $(CXXFLAGS) $(WARNFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile source/ -> build/tests/source/...
$(BUILD_DIR)/source/%.o: $(SRC_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile tests/ -> build/tests/tests/...
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile third-party C++ -> build/tests/third-party/...
$(BUILD_DIR)/third-party/%.cpp.o: $(THIRD_PARTY_DIR)/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile third-party C -> build/tests/third-party/...
$(BUILD_DIR)/third-party/%.c.o: $(THIRD_PARTY_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Sanity helper
list:
	@echo "Sources:"; echo "$(SRC_FILES)" | tr ' ' '\n'
	@echo
	@echo "Tests:";   echo "$(TEST_FILES)" | tr ' ' '\n'
	@echo
	@echo "Third-party:"; echo "$(THIRD_PARTY_FILES)" | tr ' ' '\n'
	@echo
	@echo "Objects:"; echo "$(OBJ_FILES)" | tr ' ' '\n'

clean:
	rm -rf build

# Auto-deps
-include $(DEP_FILES)
