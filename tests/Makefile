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
BUILD_DIR := ../tests/build/
BIN       := $(BUILD_DIR)/run_tests

# create the relative path to the SDL library
SDL_DIR := ../third-party/SDL2

CATCH_INC := third-party/Catch/single_include

CXXFLAGS ?= -O2 -g
WARNFLAGS ?= -Wall -Wextra -Wpedantic
CPPFLAGS += -I$(SRC_DIR) -I$(CATCH_INC) -I$(SDL_DIR)/include	# added SDL
LDFLAGS  ?=
LDLIBS   += -L$(SDL_DIR)/lib -lSDL2 -lSDL2_image -lSDL2_ttf	# added SDL

# Pick up .cpp from tests/
TEST_FILES := $(shell find $(TEST_DIR) -type f -name '*.cpp')

# Pick up headers from source/, then compile only matching .cpp next to them
SRC_HPP    := $(shell find $(SRC_DIR)  -type f -name '*.hpp')
SRC_FILES  := $(patsubst %.hpp,%.cpp,$(SRC_HPP))
SRC_FILES  := $(filter $(shell find $(SRC_DIR) -type f -name '*.cpp'),$(SRC_FILES))

OBJ_FILES := \
  $(patsubst $(TEST_DIR)/%,$(BUILD_DIR)/tests/%.o,$(TEST_FILES)) \
  $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/source/%.o,$(SRC_FILES))

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

# SDL2 Installation Target
SDL_VERSION = 2.30.0
SDL_IMAGE_VERSION = 2.8.2
SDL_TTF_VERSION = 2.22.0

install:
	@echo "Installing SDL2 libraries..."

	# Create directories
	@mkdir -p third-party/SDL2/lib
	@mkdir -p third-party/SDL2/include

	# Check if Homebrew SDL2 is installed
	@if command -v brew >/dev/null 2>&1; then \
		echo "Found Homebrew, copying SDL2 libraries..."; \
		if brew list sdl2 >/dev/null 2>&1; then \
			cp -n /opt/homebrew/Cellar/sdl2/*/lib/*.dylib third-party/SDL2/lib/ 2>/dev/null || \
			cp -n /usr/local/lib/libSDL2*.dylib third-party/SDL2/lib/ 2>/dev/null || true; \
		else \
			echo "SDL2 not found, installing via Homebrew..."; \
			brew install sdl2; \
			cp /opt/homebrew/Cellar/sdl2/*/lib/*.dylib third-party/SDL2/lib/; \
		fi; \
		if brew list sdl2_image >/dev/null 2>&1; then \
			cp -n /opt/homebrew/Cellar/sdl2_image/*/lib/*.dylib third-party/SDL2/lib/ 2>/dev/null || \
			cp -n /usr/local/lib/libSDL2_image*.dylib third-party/SDL2/lib/ 2>/dev/null || true; \
		else \
			echo "SDL2_image not found, installing via Homebrew..."; \
			brew install sdl2_image; \
			cp /opt/homebrew/Cellar/sdl2_image/*/lib/*.dylib third-party/SDL2/lib/; \
		fi; \
		if brew list sdl2_ttf >/dev/null 2>&1; then \
			cp -n /opt/homebrew/Cellar/sdl2_ttf/*/lib/*.dylib third-party/SDL2/lib/ 2>/dev/null || \
			cp -n /usr/local/lib/libSDL2_ttf*.dylib third-party/SDL2/lib/ 2>/dev/null || true; \
		else \
			echo "SDL2_ttf not found, installing via Homebrew..."; \
			brew install sdl2_ttf; \
			cp /opt/homebrew/Cellar/sdl2_ttf/*/lib/*.dylib third-party/SDL2/lib/; \
		fi; \
	else \
		echo "ERROR: Homebrew not found. Please install Homebrew first."; \
		echo "Visit: https://brew.sh"; \
		exit 1; \
	fi

	@echo "SDL2 installation complete!"
	@ls -lh third-party/SDL2/lib/*.dylib
