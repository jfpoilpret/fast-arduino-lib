#   Copyright 2016-2017 Jean-Francois Poilpret
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
# This must be included by your own Makefile for building a project
# This is actually used by all fastarduino examples

# Find path of this Makefile before including other makefiles in the same path
thispath:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(thispath)Makefile-config.mk
include $(thispath)Makefile-common.mk

# Main target project using FastArduino
$(target): $(objects) $(libs)
	$(rm) $@ $@.eep $@.nm $@.map
	$(link.o) $(abspath $^)
	$(objcopy) -R .eeprom -O ihex $@ $@.hex
	$(objcopy) -j .eeprom --change-section-lma .eeprom=0 -O ihex $@ $@.eep
	$(nm) --synthetic -S -C --size-sort $@ >$@.nm
	$(objdump) -m $(ARCH) -x -d -C $@ >$@.dump
	$(objsize) -C --mcu=$(MCU) $@

# Upload Targets
.PHONY: .upload-check
.upload-check:
ifneq ($(can_upload),true)
	$(info Missing configuration variables for upload.)
	$(info Either PROGRAMMER (and optionally COM) is provided)
	$(info or the following variables:)
	$(info - DUDE_OPTION (mandatory)
	$(info - DUDE_SERIAL (needed for some programmers)
	$(info - DUDE_SERIAL_RESET (needed for some programmers)
	$(info - CAN_PROGRAM_EEPROM=true (for some programmers)
	$(info - CAN_PROGRAM_FUSES=true (for some programmers)
	$(error Cannot proceed)
endif

.PHONY: .fuses-check
.fuses-check: .upload-check
ifneq ($(has_fuses),true)
	$(info Missing configuration variables for fuses.)
	$(info The following variables must be provided (in 0xHH format):)
	$(info - HFUSE)
	$(info - LFUSE)
	$(info - EFUSE)
	$(error Cannot proceed)
endif

# Handle specific preparation for LEONARDO
.PHONY: .pre-upload
.pre-upload:
ifneq ($(DUDE_SERIAL_RESET),)
	stty -F $(DUDE_SERIAL_RESET) ispeed 1200 1200 || :
	sleep 2.5
endif

.PHONY: flash
flash:  $(target) .upload-check .pre-upload
	avrdude $(avrdude_options) -Uflash:w:$<.hex:i

.PHONY: eeprom
eeprom:  $(target) .upload-check .pre-upload
ifeq ($(CAN_PROGRAM_EEPROM),true)
	avrdude $(avrdude_options) -D -U eeprom:w:$<.eep:i
else
	$(error EEPROM cannot be written for that target and programmer)
endif

.PHONY: fuses
fuses: .fuses-check .pre-upload
ifeq ($(CAN_PROGRAM_FUSES),true)
	avrdude $(avrdude_options) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m
else
	$(error Fuses cannot be written for that target and programmer)
endif

-include $(deps)

