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

# This file is included by Makefile-lib and Makefile-app to allow simpler command-line usage
# by providing only a configuration (with special naming convention) which will then be
# converted to all variables expected by Makefile-common

# set variables based on each configuration
ifeq ($(findstring UNO,$(CONF)),UNO)
	VARIANT:=ARDUINO_UNO
	MCU:=atmega328p
	ARCH:=avr5
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=UNO
	endif
	ifeq ($(COM),)
		COM:=/dev/ttyACM0
	endif
else
ifeq ($(findstring NANO,$(CONF)),NANO)
	VARIANT:=ARDUINO_NANO
	MCU:=atmega328p
	ARCH:=avr5
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=NANO
	endif
	ifeq ($(COM),)
		COM:=/dev/ttyUSB0
	endif
else
ifeq ($(findstring LEONARDO,$(CONF)),LEONARDO)
	VARIANT:=ARDUINO_LEONARDO
	MCU:=atmega32u4
	ARCH:=avr5
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=LEONARDO
	endif
	#TODO Improve: deduce one from the other
	ifeq ($(COM_LEONARDO),)
		COM_LEONARDO:=/dev/ttyACM0
	endif
	ifeq ($(COM),)
		COM:=/dev/ttyACM1
	endif
else
ifeq ($(findstring ATmega328,$(CONF)),ATmega328)
	VARIANT:=BREADBOARD_ATMEGA328P
	MCU:=atmega328p
	ARCH:=avr5
	F_CPU:=8000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else
ifeq ($(findstring ATtiny84,$(CONF)),ATtiny84)
	VARIANT:=BREADBOARD_ATTINYX4
	MCU:=attiny84
	ARCH:=avr25
	F_CPU:=8000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else
ifeq ($(findstring MEGA,$(CONF)),MEGA)
	VARIANT:=ARDUINO_MEGA
	MCU:=atmega2560
	ARCH:=avr6
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=MEGA
	endif
	ifeq ($(COM),)
		COM:=/dev/ttyACM0
	endif
# Add other targets here
endif
endif
endif
endif
endif
endif

# set F_CPU if config name includes it (eg 8MHz, 16MHz)
ifeq ($(findstring 16MHz,$(CONF)),16MHz)
	F_CPU:=16000000UL
else
ifeq ($(findstring 8MHz,$(CONF)),8MHz)
	F_CPU:=8000000UL
else
ifeq ($(findstring 20MHz,$(CONF)),20MHz)
	F_CPU:=20000000UL
endif
endif
endif

# Set upload options
ifeq ($(PROGRAMMER),)
	PROGRAMMER=UNO
endif
AVRDUDE_OPTIONS=-p $(MCU)
ifeq ($(PROGRAMMER),ISP)
        AVRDUDE_OPTIONS+= -c arduinoisp 
endif
ifeq ($(PROGRAMMER),SHIELD)
        AVRDUDE_OPTIONS+= -c stk500v1 -b 19200 -P $(COM)
endif
ifeq ($(PROGRAMMER),UNO)
        AVRDUDE_OPTIONS+= -c arduino -b 115200 -P $(COM)
endif
ifeq ($(PROGRAMMER),NANO)
        AVRDUDE_OPTIONS+= -c arduino -b 57600 -P $(COM)
endif
ifeq ($(PROGRAMMER),LEONARDO)
        AVRDUDE_OPTIONS+= -c avr109 -b 57600 -P $(COM)
endif
ifeq ($(PROGRAMMER),MEGA)
        AVRDUDE_OPTIONS+= -c wiring -b 115200 -P $(COM)
endif

