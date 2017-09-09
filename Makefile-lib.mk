# Makefile inspired by https://gist.github.com/maxtruxa/4b3929e118914ccef057f8a05c614b0f
# Adapted for FastArduino library and projects
#
# This must be included by your own Makefile for building a library
# This is actually used for fastarduino library itself.
#

# Find path of this Makefile before including other makefiles in the same path
thispath:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
#include $(thispath)/Makefile-config.mk
include $(thispath)/Makefile-common.mk

# Main target library
$(target): $(objects) $(libs)
	$(rm) $@
	$(ar) -rv $@ $^
	$(ranlib) $@

-include $(deps)

