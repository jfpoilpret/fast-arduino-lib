# Makefile inspired by https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Adapted for FastArduino library and projects
#
# This must be included by your own Makefile for building a library
# This is actually used for fastarduino library itself.
#

#include Makefile-config.mk
include Makefile-common.mk

# Main target library
$(target): $(objects) $(TARGET_LIBS)
	$(rm) $@
	$(ar) -rv $@ $^
	$(ranlib) $@

-include $(deps)

