# Top-level Makefile (repo root)
#
# Delegates compatibility targets to:
#   source/   (CMake-backed program builds)
#   tests/    (CMake-backed native and WebUI tests)
#
# Common usage:
#   make                # build default program(s)
#   make test           # build+run unit tests
#   make all            # build program(s) + run tests
#   make clean          # clean both
#   make debug          # debug build (programs)
#   make opt            # optimized build (programs)
#
# You can also forward any target directly:
#   make src-simple
#   make src-debug-simple
#   make test-build
#   make test-list

# The Make targets below are compatibility shims. Compilation and test
# orchestration now live in the root CMake project.

.PHONY: default all build test clean debug opt quick grumpy \
        src-% test-% help web_test group18_main

# ---------- High-level targets ----------

default: build

# Build program(s) (uses source/ Makefile "default" target)
build:
	$(MAKE) -C source

# Build + run unit tests (uses tests/ Makefile "test" target)
test:
	$(MAKE) -C tests test

# Build program(s) + run tests
all: build test

# Program build variants (forwarded to source/)
debug opt quick grumpy:
	$(MAKE) -C source $@

# Clean everything
clean:
	$(MAKE) -C source clean
	$(MAKE) -C tests clean

# Forward anything to source/ by prefixing with src-
#   make src-simple
#   make src-debug
#   make src-opt-simple
src-%:
	$(MAKE) -C source $*

# Forward anything to tests/ by prefixing with test-
#   make test-build
#   make test-test
#   make test-list
test-%:
	$(MAKE) -C tests $*

# Build + run WebUI unit tests (requires Emscripten SDK / em++)
web_test:
	$(MAKE) -C tests web_test

group18_main:
	$(MAKE) -C source group18_main

help:
	@echo "Top-level targets:"
	@echo "  make / make build      Build program(s) via CMake"
	@echo "  make test              Build + run unit tests via CMake"
	@echo "  make web_test          Build WebUI tests with em++ (requires Emscripten)"
	@echo "  make all               Build program(s) + run tests"
	@echo "  make debug|opt|quick|grumpy   Build program(s) with that mode"
	@echo "  make clean             Clean source/, tests/, and build/cmake/"
	@echo
	@echo "Forwarding targets:"
	@echo "  make src-<tgt>         Run 'make <tgt>' in source/"
	@echo "  make test-<tgt>        Run 'make <tgt>' in tests/"

# ---------- SDL INSTALL SECTION ----------

SDL_VERSION       = 2.32.10
SDL_IMAGE_VERSION = 2.8.8
SDL_TTF_VERSION   = 2.24.0

SDL_PREFIX = $(CURDIR)/third-party/SDL2
SDL_BUILD  = $(CURDIR)/third-party/sdl-build
SDL_STAMP  = $(SDL_PREFIX)/.installed

UNAME_S := $(shell uname -s)

.PHONY: install install-clean

install: $(SDL_STAMP)

$(SDL_STAMP):
	@echo "Installing SDL locally into third-party/SDL2..."
	@mkdir -p $(SDL_BUILD)
	@mkdir -p $(SDL_PREFIX)

	@echo "Downloading SDL2..."
	@if [ ! -f $(SDL_BUILD)/SDL2-$(SDL_VERSION).tar.gz ]; then \
		curl -L -o $(SDL_BUILD)/SDL2-$(SDL_VERSION).tar.gz \
		https://github.com/libsdl-org/SDL/releases/download/release-$(SDL_VERSION)/SDL2-$(SDL_VERSION).tar.gz; \
	fi

	@echo "Extracting SDL2..."
	@if [ ! -d $(SDL_BUILD)/SDL2-$(SDL_VERSION) ]; then \
		tar -xzf $(SDL_BUILD)/SDL2-$(SDL_VERSION).tar.gz -C $(SDL_BUILD); \
	fi

	@echo "Building SDL2..."
	cd $(SDL_BUILD)/SDL2-$(SDL_VERSION) && \
	./configure --prefix=$(SDL_PREFIX) && \
	$(MAKE) -j4 && \
	$(MAKE) install

	@echo "Downloading SDL2_image..."
	@if [ ! -f $(SDL_BUILD)/SDL2_image-$(SDL_IMAGE_VERSION).tar.gz ]; then \
		curl -L -o $(SDL_BUILD)/SDL2_image-$(SDL_IMAGE_VERSION).tar.gz \
		https://github.com/libsdl-org/SDL_image/releases/download/release-$(SDL_IMAGE_VERSION)/SDL2_image-$(SDL_IMAGE_VERSION).tar.gz; \
	fi

	@echo "Extracting SDL2_image..."
	@if [ ! -d $(SDL_BUILD)/SDL2_image-$(SDL_IMAGE_VERSION) ]; then \
		tar -xzf $(SDL_BUILD)/SDL2_image-$(SDL_IMAGE_VERSION).tar.gz -C $(SDL_BUILD); \
	fi

	@echo "Building SDL2_image..."
	cd $(SDL_BUILD)/SDL2_image-$(SDL_IMAGE_VERSION) && \
	./configure --prefix=$(SDL_PREFIX) && \
	$(MAKE) -j4 && \
	$(MAKE) install

	@echo "Downloading SDL2_ttf..."
	@if [ ! -f $(SDL_BUILD)/SDL2_ttf-$(SDL_TTF_VERSION).tar.gz ]; then \
		curl -L -o $(SDL_BUILD)/SDL2_ttf-$(SDL_TTF_VERSION).tar.gz \
		https://github.com/libsdl-org/SDL_ttf/releases/download/release-$(SDL_TTF_VERSION)/SDL2_ttf-$(SDL_TTF_VERSION).tar.gz; \
	fi

	@echo "Extracting SDL2_ttf..."
	@if [ ! -d $(SDL_BUILD)/SDL2_ttf-$(SDL_TTF_VERSION) ]; then \
		tar -xzf $(SDL_BUILD)/SDL2_ttf-$(SDL_TTF_VERSION).tar.gz -C $(SDL_BUILD); \
	fi

	@echo "Building SDL2_ttf..."
	cd $(SDL_BUILD)/SDL2_ttf-$(SDL_TTF_VERSION) && \
	./configure --prefix=$(SDL_PREFIX) && \
	$(MAKE) -j4 && \
	$(MAKE) install

	@touch $(SDL_STAMP)
	@echo "SDL installation complete."

install-clean:
	rm -rf third-party/sdl-build
	rm -rf third-party/SDL2
