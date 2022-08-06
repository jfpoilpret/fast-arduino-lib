#
#   Copyright 2016-2022 Jean-Francois Poilpret
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
 
# Set necessary variables for generic makefile
# Main target name (.a for library, no extension for app) without path
TARGET:=libfastarduino.a
# Which directory to look for source files (.cpp)
SOURCE_ROOT:=cores
# Comma-separated list of directories containing external headers to include
ADDITIONAL_INCLUDES:=
# Space-separated list of paths to libraries to link with target app
ADDITIONAL_LIBS:=

# include generic makefile for libs
include make/Makefile-lib.mk

# FastArduino doc generation targets
# - docs is target for html doc (published on github pages)
# - apidoc is target for all other documentation formats (LATEX)
# - re-create all dirs and subdirs as doxygen cannot properly create them
# - call each generation
doco:
	mkdir -p apidoc
	mkdir -p apidoc/latex
	mkdir -p apidoc/latex/boards
	mkdir -p docs/boards
	doxygen ./dox/doxyfile-api
	doxygen ./dox/doxyfile-uno
	doxygen ./dox/doxyfile-nano
	doxygen ./dox/doxyfile-leonardo
	doxygen ./dox/doxyfile-mega
	doxygen ./dox/doxyfile-atmega328
	doxygen ./dox/doxyfile-atmegaxx4
	doxygen ./dox/doxyfile-attinyx4
	doxygen ./dox/doxyfile-attinyx5

# include list of all examples
include make/Makefile-Examples.mk

