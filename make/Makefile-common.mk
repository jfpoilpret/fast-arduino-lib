#   Copyright 2016-2021 Jean-Francois Poilpret
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

# Makefile inspired by https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Adapted for FastArduino library and projects
#
# This is included by either Makefile-lib or Makefile-app
#

# Set flags indicating if we have enough configuration setup for targets
can_build:=$(and $(VARIANT),$(ARCH),$(MCU),$(F_CPU),true)
can_upload:=$(and $(DUDE_OPTION),$(can_build))
has_fuses:=$(and $(LFUSE),$(HFUSE),$(EFUSE),true)

# Explicitly set path to AVR-GCC binaries if needed (default is to use $PATH)
ifndef AVR_TOOL_PATH
	avr_tool_path:=
else
	# Add / at the end if needed
	avr_tool_path:=$(realpath $(AVR_TOOL_PATH))/
endif

# Prepare additional compiler options (optional)
ifndef ADDITIONAL_CXX_OPTIONS
	ADDITIONAL_CXX_OPTIONS:=
endif

# Prepare additional linker options (optional)
ifndef ADDITIONAL_LD_OPTIONS
	ADDITIONAL_LD_OPTIONS:=
endif

# Output directories
config:=$(VARIANT)-$(subst 000000UL,MHz,$(F_CPU))
objdir:=build/$(config)
depdir:=deps/$(config)
distdir:=dist/$(config)
target:=$(distdir)/$(TARGET)

# Libraries dependencies
ifeq ($(FASTARDUINO_ROOT),)
	FASTARDUINO_ROOT := .
	fastarduinolib:=
else
	fastarduinolib:=$(FASTARDUINO_ROOT)/dist/$(config)/libfastarduino.a
endif
libs:=$(fastarduinolib) $(ADDITIONAL_LIBS)

# Input directories
includes:=$(patsubst %,-I %,$(abspath $(FASTARDUINO_ROOT)/cores $(ADDITIONAL_INCLUDES)))

# List of source files
sources:=$(shell find $(SOURCE_ROOT) -name "*.cpp")
# List of generated files: objects and dependencies
objects:=$(patsubst %,$(objdir)/%.o,$(basename $(sources)))
deps:=$(patsubst %,$(depdir)/%.d,$(basename $(sources)))

# Create all needed subdirectories
ifeq ($(can_build), true)
    $(shell mkdir -p $(dir $(objects)) >/dev/null)
    $(shell mkdir -p $(dir $(deps)) >/dev/null)
    $(shell mkdir -p $(distdir) >/dev/null)
endif

# Environment
rm:=rm -f
ar:=$(avr_tool_path)avr-gcc-ar
ranlib:=$(avr_tool_path)avr-gcc-ranlib
cxx:=$(avr_tool_path)avr-g++
nm:=$(avr_tool_path)avr-nm
objcopy:=$(avr_tool_path)avr-objcopy
objdump:=$(avr_tool_path)avr-objdump
objsize:=$(avr_tool_path)avr-size

# Flags for compilation and build
#NOTE initial common flags (9.2)
commonflags:= -mmcu=$(MCU) -DF_CPU=$(F_CPU) -D$(VARIANT) -std=c++17 -Wall -Wextra -Os -fno-exceptions -flto -felide-constructors -ffunction-sections -fdata-sections -mcall-prologues

# Additional flags (for testing optimization) can be set to the following variable used with GCC 10.2
extraflags:= --param=max-inline-insns-auto=20 --param=max-inline-insns-single=20 --param=early-inlining-insns=20

cxxflags:= $(commonflags) $(extraflags) -DNO_ABI $(includes) -g0
ldflags = $(commonflags) $(extraflags) -Wl,--gc-sections -Wl,--relax -Wl,-Map,$@.map
depflags = -MT $@ -MMD -MP -MF $(depdir)/$*.Td

compile.cc = $(cxx) $(depflags) $(cxxflags) $(ADDITIONAL_CXX_OPTIONS) -c -o $@
precompile =
postcompile = mv -f $(depdir)/$*.Td $(depdir)/$*.d
link.o = $(cxx) $(ldflags) $(ADDITIONAL_LD_OPTIONS) -o $@

# Flags for target programming (upload)
avrdude_options:=-p $(MCU) $(DUDE_OPTION) $(if $(DUDE_SERIAL),-P $(DUDE_SERIAL))

.PHONY: build
build: .build-check $(target)

# Ensure that we have enough config for running make
.PHONY: .build-check
.build-check:
ifneq ($(can_build), true)
	$(info Missing configuration variables.)
	$(info Either CONF is provided or all of the following variables:)
	$(info - VARIANT)
	$(info - MCU)
	$(info - F_CPU)
	$(info - ARCH)
	$(error Cannot proceed)
endif

.PHONY: clean
clean: .build-check
	$(rm) -r $(objdir) $(depdir) $(distdir)

.PHONY: clean-all
clean-all:
	$(rm) -r build/* deps/* dist/*

.PHONY: help
help:
	@echo Available targets: build clean

# Target for generating dependencies
$(objdir)/%.o: %.cpp
$(objdir)/%.o: %.cpp $(depdir)/%.d
	$(precompile)
	$(compile.cc) $(abspath $<)
	$(postcompile)

.PRECIOUS = $(depdir)/%.d
$(depdir)/%.d: ;

