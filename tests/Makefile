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

PLATFORM  := native

SRC_DIR   := ../source
TEST_DIR  := ../tests
BUILD_DIR := ../tests/build/$(PLATFORM)
BIN       := $(BUILD_DIR)/run_tests

CATCH_INC := ../third-party/Catch/single_include

CXX       := c++
CXXFLAGS  ?= -O2 -g
WARNFLAGS ?= -Wall
CPPFLAGS  += -I$(SRC_DIR) -I$(CATCH_INC)
LDFLAGS   ?=
LDLIBS    ?=

# Pick up .cpp from tests/
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp')

# Pick up headers from source/, then compile only matching .cpp next to them
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp'),$(SRC_FILES))
SRC_FILES  := $(filter-out %main.cpp %Main.cpp %WebCanvasDemo.cpp,$(SRC_FILES))

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES))

DEP_FILES := $(OBJ_FILES:.o=.d)

ifeq ($(PLATFORM), web)
	BIN     := $(BUILD_DIR)/WebUi_tests.js
	TARGET  := $(TEST_DIR)/webui_testrunner.js
	CXX     := em++
	LIBRARY := $(SRC_DIR)/Interfaces/WebUI/WebCanvas/WebCanvas.js
	LDFLAGS += --bind -sALLOW_MEMORY_GROWTH=1 -sASSERTIONS=1 --js-library $(LIBRARY) -sINVOKE_RUN=1 -fexceptions
endif

.PHONY: all build test clean list

all: test

build: $(BIN)

test: $(BIN)
ifeq ($(PLATFORM), web)
	node $(TARGET)
else
	$(BIN)
endif

# Link test runner
$(BIN): $(OBJ_FILES)
	$(CXX) -std=c++23 $(CXXFLAGS) $(WARNFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile source/ -> build/tests/source/...
$(BUILD_DIR)/source/%.o: $(SRC_DIR)/%
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

# Compile tests/ -> build/tests/tests/...
$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%
	@mkdir -p $(dir $@)
	$(CXX) -std=c++23 $(CPPFLAGS) $(CXXFLAGS) $(WARNFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Sanity helper
list:
	@echo "Sources:"; echo "$(SRC_FILES)" | tr ' ' '\n'
	@echo
	@echo "Tests:";   echo "$(TEST_FILES)" | tr ' ' '\n'
	@echo
	@echo "Objects:"; echo "$(OBJ_FILES)" | tr ' ' '\n'

clean:
	rm -rf build

# Auto-deps
-include $(DEP_FILES)
