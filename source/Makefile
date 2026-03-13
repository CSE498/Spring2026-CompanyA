ROOT_DIR := $(abspath ..)
BUILD_ROOT := $(ROOT_DIR)/build/cmake
DEFAULT_BUILD_DIR := $(BUILD_ROOT)/default
DEBUG_BUILD_DIR := $(BUILD_ROOT)/debug
OPT_BUILD_DIR := $(BUILD_ROOT)/opt
QUICK_BUILD_DIR := $(BUILD_ROOT)/quick
GRUMPY_BUILD_DIR := $(BUILD_ROOT)/grumpy
CMAKE ?= cmake

.PHONY: default all clean debug opt quick grumpy simple \
	debug-simple opt-simple quick-simple grumpy-simple help

define configure_profile
	$(CMAKE) -S $(ROOT_DIR) -B $(BUILD_ROOT)/$(1) -DCOMPANYA_PROFILE=$(1)
endef

define build_profile
	$(call configure_profile,$(1))
	$(CMAKE) --build $(BUILD_ROOT)/$(1) --target simple
endef

default: all

all simple:
	$(call build_profile,default)

debug debug-simple:
	$(call build_profile,debug)

opt opt-simple:
	$(call build_profile,opt)

quick quick-simple:
	$(call build_profile,quick)

grumpy grumpy-simple:
	$(call build_profile,grumpy)

clean:
	$(CMAKE) -E rm -rf $(BUILD_ROOT)
	$(CMAKE) -E rm -f $(ROOT_DIR)/source/simple

help:
	@echo "Source targets:"
	@echo "  make / make all / make simple"
	@echo "  make debug|opt|quick|grumpy"
	@echo "  make debug-simple|opt-simple|quick-simple|grumpy-simple"
	@echo "  make clean"
