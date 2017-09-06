#
# Makefile inspired by https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Adapted for FastArduino library and projects
#
# This must be included by your own Makefile for building a project
# This is actually used by all fastarduino examples

#include Makefile-config.mk
include Makefile-common.mk

# Main target project using FastArduino
$(target): $(objects) $(TARGET_LIBS)
	$(rm) $@ $@.eep $@.nm $@.map
	$(link.o) $^
	$(objcopy) -R .eeprom -O ihex $@ $@.hex
	$(objcopy) -j .eeprom --change-section-lma .eeprom=0 -O ihex $@ $@.eep
	$(nm) --synthetic -S -C --size-sort $@ >$@.nm
	$(objdump) -m $(ARCH) -x -d -C $@ >$@.dump
	$(objsize) -C --mcu=$(MCU) $@

-include $(deps)

