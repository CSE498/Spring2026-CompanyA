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

# create the relative path to the SDL library
SDL_DIR := $(THIRD_PARTY_DIR)/SDL2

CATCH_INC := $(THIRD_PARTY_DIR)/Catch/single_include
THIRD_PARTY_C_FILES := $(THIRD_PARTY_DIR)/Tinyexpr/tinyexpr.c

CXXFLAGS ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic -Werror
CC ?= cc
CFLAGS ?= -O2 -g
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC) -I$(SDL_DIR)/include
LDFLAGS  += -Wl,-rpath,'$$ORIGIN/../../third-party/SDL2/lib'
LDLIBS   += -L$(SDL_DIR)/lib -lSDL2 -lSDL2_image -lSDL2_ttf	# added SDL

# Pick up .cpp from tests/ (exclude WebUI — those require Emscripten/em++)
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp' -not -path '*/WebUI/*')

# Pick up headers from source/, then compile only matching .cpp next to them
# (exclude WebUI — headers pull in <emscripten/val.h>)
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp' -not -path '*/WebUI/*')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp' -not -path '*/WebUI/*'),$(SRC_FILES))

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES)) \
  $(patsubst $(THIRD_PARTY_DIR)/%,$(BUILD_DIR)/third-party/%.o,$(THIRD_PARTY_C_FILES))

DEP_FILES := $(OBJ_FILES:.o=.d)

.PHONY: all build test clean list install

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

# Compile third-party C sources -> build/tests/third-party/...
$(BUILD_DIR)/third-party/%.o: $(THIRD_PARTY_DIR)/% | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) -std=c11 $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

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
	rm -rf $(BUILD_DIR)

# Auto-deps
-include $(DEP_FILES)

# ---------- WebUI tests (require Emscripten SDK / em++) ----------
#
# Usage:  make web_test          (from this directory)
#         make test-web_test     (from the repo root)
#
# Prerequisites:
#   1. Install & activate the Emscripten SDK  (emsdk install latest && emsdk activate latest)
#   2. Source the environment                  (source emsdk_env.sh)
#
# After building, serve the HTML files and open them in a browser:
#   cd build/web_tests && python3 -m http.server 8000
#   Then visit http://localhost:8000/<TestName>.html

EMCC       ?= ../third-party/emsdk/upstream/emscripten/em++
EM_CXXFLAGS = --bind -std=c++23 -s WASM=1 -s ASSERTIONS=1 -DCATCH_CONFIG_MAIN -gsource-map

WEB_TEST_OUT = $(BUILD_DIR)/web_tests
WEB_SRC      = $(SRC_DIR)/Interfaces/WebUI
WEB_TEST     = $(TEST_DIR)/Interfaces/WebUI
WEB_CANVAS_CPP = $(WEB_SRC)/WebCanvas/WebCanvas.cpp
WEB_CANVAS_JS  = --js-library ../source/Interfaces/WebUI/WebCanvas/WebCanvas.js

.PHONY: web_test

web_test:
	@mkdir -p $(WEB_TEST_OUT)
	$(EMCC) $(EM_CXXFLAGS) -I$(SRC_DIR) -I$(CATCH_INC) \
		$(WEB_TEST)/WebTextboxTest.cpp \
		$(WEB_SRC)/WebTextbox/WebTextbox.cpp \
		$(WEB_SRC)/WebLayout/WebLayout.cpp \
		$(WEB_CANVAS_CPP) \
		$(WEB_CANVAS_JS) \
		-o $(WEB_TEST_OUT)/WebTextboxTest.html
	$(EMCC) $(EM_CXXFLAGS) -I$(SRC_DIR) -I$(CATCH_INC) \
		$(WEB_TEST)/WebImageTest.cpp \
		$(WEB_SRC)/WebImage/WebImage.cpp \
		$(WEB_SRC)/WebLayout/WebLayout.cpp \
		$(WEB_CANVAS_CPP) \
		$(WEB_CANVAS_JS) \
		-o $(WEB_TEST_OUT)/WebImageTest.html
	$(EMCC) $(EM_CXXFLAGS) -I$(SRC_DIR) -I$(CATCH_INC) \
		$(WEB_TEST)/WebButtonTest.cpp \
		$(WEB_SRC)/WebButton/WebButton.cpp \
		$(WEB_SRC)/WebLayout/WebLayout.cpp \
		$(WEB_CANVAS_CPP) \
		$(WEB_CANVAS_JS) \
		-o $(WEB_TEST_OUT)/WebButtonTest.html
	$(EMCC) $(EM_CXXFLAGS) -I$(SRC_DIR) -I$(CATCH_INC) \
		$(WEB_TEST)/WebCanvasTest.cpp \
		$(WEB_SRC)/WebLayout/WebLayout.cpp \
		$(WEB_CANVAS_CPP) \
		$(WEB_CANVAS_JS) \
		-o $(WEB_TEST_OUT)/WebCanvasTest.html
	$(EMCC) $(EM_CXXFLAGS) -I$(SRC_DIR) -I$(CATCH_INC) \
		$(WEB_TEST)/WebLayoutTest.cpp \
		$(WEB_SRC)/WebLayout/WebLayout.cpp \
		-o $(WEB_TEST_OUT)/WebLayoutTest.html
	@echo ""
	@echo "WebUI tests compiled to $(WEB_TEST_OUT)/"
	@echo "To run:  cd $(WEB_TEST_OUT) && python3 -m http.server 8000"
	@echo "Then open http://localhost:8000/<TestName>.html in your browser."
