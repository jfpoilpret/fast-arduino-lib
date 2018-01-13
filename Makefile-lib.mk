#   Copyright 2016-2018 Jean-Francois Poilpret
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
# This must be included by your own Makefile for building a library
# This is actually used for fastarduino library itself.
#

# Find path of this Makefile before including other makefiles in the same path
thispath:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
include $(thispath)Makefile-config.mk
include $(thispath)Makefile-common.mk

# Main target library
$(target): $(objects) $(libs)
	$(rm) $@
	$(ar) -rv $@ $^
	$(ranlib) $@

-include $(deps)

