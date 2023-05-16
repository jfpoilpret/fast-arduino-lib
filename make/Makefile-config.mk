#   Copyright 2016-2023 Jean-Francois Poilpret
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
else ifeq ($(findstring NANO,$(CONF)),NANO)
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
else ifeq ($(findstring LEONARDO,$(CONF)),LEONARDO)
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
else ifeq ($(findstring ATmega328,$(CONF)),ATmega328)
	VARIANT:=BREADBOARD_ATMEGA328P
	MCU:=atmega328p
	ARCH:=avr5
	F_CPU:=8000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else ifeq ($(findstring ATmega644,$(CONF)),ATmega644)
	VARIANT:=BREADBOARD_ATMEGAXX4P
	MCU:=atmega644p
	ARCH:=avr5
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else ifeq ($(findstring ATmega1284,$(CONF)),ATmega1284)
	VARIANT:=BREADBOARD_ATMEGAXX4P
	MCU:=atmega1284p
	ARCH:=avr5
	F_CPU:=16000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else ifeq ($(findstring ATtiny84,$(CONF)),ATtiny84)
	VARIANT:=BREADBOARD_ATTINYX4
	MCU:=attiny84
	ARCH:=avr25
	F_CPU:=8000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else ifeq ($(findstring ATtiny85,$(CONF)),ATtiny85)
	VARIANT:=BREADBOARD_ATTINYX5
	MCU:=attiny85
	ARCH:=avr25
	F_CPU:=8000000UL
	ifeq ($(PROGRAMMER),)
		PROGRAMMER:=ISP
	endif
else ifeq ($(findstring MEGA,$(CONF)),MEGA)
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

# set F_CPU if config name includes it (eg 8MHz, 16MHz)
ifeq ($(findstring 16MHz,$(CONF)),16MHz)
	F_CPU:=16000000UL
else ifeq ($(findstring 8MHz,$(CONF)),8MHz)
	F_CPU:=8000000UL
else ifeq ($(findstring 20MHz,$(CONF)),20MHz)
	F_CPU:=20000000UL
endif

# Set upload options
ifndef DUDE_OPTION
	ifeq ($(PROGRAMMER),ISP)
        	DUDE_OPTION := -c arduinoisp 
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),ISPORG)
        	DUDE_OPTION := -c arduinoisporg
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),TINYISP)
        	DUDE_OPTION := -c usbtiny
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),SHIELD)
		DUDE_OPTION := -c stk500v1 -b 19200
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),UNO)
		DUDE_OPTION := -c arduino -b 115200
	else ifeq ($(PROGRAMMER),NANO)
		DUDE_OPTION := -c arduino -b 57600
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),LEONARDO)
		DUDE_OPTION := -c avr109 -b 57600
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	else ifeq ($(PROGRAMMER),MEGA)
		DUDE_OPTION := -c wiring -b 115200
		CAN_PROGRAM_EEPROM := true
		CAN_PROGRAM_FUSES := true
	endif
endif

ifeq ($(origin DUDE_SERIAL),undefined)
	DUDE_SERIAL := $(COM)
endif

ifeq ($(origin DUDE_SERIAL_RESET),undefined)
	DUDE_SERIAL_RESET := $(COM_LEONARDO)
endif

#$(info $$VARIANT is [${VARIANT}])
#$(info $$ARCH is [${ARCH}])
#$(info $$MCU is [${MCU}])
#$(info $$F_CPU is [${F_CPU}])
#$(info $$DUDE_OPTION is [${DUDE_OPTION}])
#$(info $$DUDE_SERIAL is [${DUDE_SERIAL}])
#$(info $$DUDE_SERIAL_RESET is [${DUDE_SERIAL_RESET}])
#$(info $$CAN_PROGRAM_EEPROM is [${CAN_PROGRAM_EEPROM}])
#$(info $$CAN_PROGRAM_FUSES is [${CAN_PROGRAM_FUSES}])

