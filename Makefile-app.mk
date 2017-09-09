#
# Makefile inspired by https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Adapted for FastArduino library and projects
#
# This must be included by your own Makefile for building a project
# This is actually used by all fastarduino examples

# Find path of this Makefile before including other makefiles in the same path
thispath:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
#include $(thispath)/Makefile-config.mk
include $(thispath)/Makefile-common.mk

# Main target project using FastArduino
$(target): $(objects) $(libs)
	$(rm) $@ $@.eep $@.nm $@.map
	$(link.o) $^
	$(objcopy) -R .eeprom -O ihex $@ $@.hex
	$(objcopy) -j .eeprom --change-section-lma .eeprom=0 -O ihex $@ $@.eep
	$(nm) --synthetic -S -C --size-sort $@ >$@.nm
	$(objdump) -m $(ARCH) -x -d -C $@ >$@.dump
	$(objsize) -C --mcu=$(MCU) $@

# Upload Targets
.pre-upload:
	# Specific preparation for LEONARDO
ifneq ($(DUDE_SERIAL_RESET),)
	stty -F $(DUDE_SERIAL_RESET) ispeed 1200 1200 || :
	sleep 2.5
endif

flash: $(target) .pre-upload
	avrdude $(avrdude_options) -Uflash:w:$<.hex:i

eeprom: $(target) .pre-upload
ifeq ($(CAN_PROGRAM_EEPROM),YES)
	avrdude $(avrdude_options) -D -U eeprom:w:$<.eep:i
else
	$(error EEPROM cannot be written for that target and programmer)
endif

#TODO fuses programming support

-include $(deps)

