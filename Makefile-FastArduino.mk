# Generic Makefile for FastArduino library, FastArduino examples and any FastArduino-based program.
# To use it, you must include it in your project Makefile by adding the following line 
# (without the '#' comment prefix):
#
# include <relative-path-to-fastarduino>/Makefile-FastArduino.mk
#
# This Makefile expects configurations names TARGET[-TYPE-][-FREQ]
# Where:
# - TARGET is one of:
#   - UNO
#   - MEGA
#   - ATmega328
#   - ATtiny84
#
# - TYPE is optional and can be anything you want to set different settings, e.g.
#   - Release
#   - Debug
#
# - FREQ is optional and allows changing default frequency values for TARGET; can be one of:
#   - 8MHz (default for ATtiny and ATmega configurations)
#   - 16MHZ
#   - 20MHz
# NB: FREQ should not be changed for standard ARduino boards, only for breadboard AVR MCU.
#
# Additional parameters are expected for uploading programs to the MCU:
# - PROGRAMMER sets the programmer used for upload, can be one of:
#   - UNO	use default UNO bootloader through USB (default for Arduino UNO)
#   - MEGA	use default MEGA bootloader through USB (default for Arduino MEGA)
#   - ISP	use ArduinoISP breakout connected to PC through USB (used by default for breadboard configs)
#   - SHIELD	use AVR ISP shield coupled with an Arduino UNO
#
# - COM sets the USB device to use to connect to a board (defaults to /dev/ttyACM0). Note that ArduinoISP
# does not need to be defined any USB device as its drivers automatically find it.
#
# This Makefile provides additional make targets that you can use from NetBeans or in command line):
# - flash	upload already built program to TARGET flash memory
# - fuses	upload fuses to TARGET (fuses are set by default based on TARGET and FREQ)
# - eeprom	upload data to TARGET eeprom memory (NOT YET IMPLEMENTED)
#
# TODO set default FUSES for TARGETS, allow FUSES as parameters

# TODO Infer reuse of Arduino setup (programmers, frequencies, architecture...)

# .build-post: .build-impl .build-exe
#
# 

# set variables based on each configuration
ifeq ($(findstring UNO,${CONF}),UNO)
	VARIANT=ARDUINO_UNO
	MCU=atmega328p
	ARCH=avr5
	F_CPU=16000000L
	ifeq (${PROGRAMMER},)
		PROGRAMMER=UNO
	endif
	ifeq (${COM},)
		COM=/dev/ttyACM0
	endif
else
ifeq ($(findstring ATmega328,${CONF}),ATmega328)
	VARIANT=BREADBOARD_ATMEGA328P
	MCU=atmega328p
	ARCH=avr5
	F_CPU=8000000L
	ifeq (${PROGRAMMER},)
		PROGRAMMER=ISP
	endif
else
ifeq ($(findstring ATtiny84,${CONF}),ATtiny84)
	VARIANT=BREADBOARD_ATTINYX4
	MCU=attiny84
	ARCH=avr25
	F_CPU=8000000L
	ifeq (${PROGRAMMER},)
		PROGRAMMER=ISP
	endif
else
ifeq ($(findstring MEGA,${CONF}),MEGA)
	VARIANT=ARDUINO_MEGA
	MCU=atmega2560
	ARCH=avr6
	F_CPU=16000000L
	ifeq (${PROGRAMMER},)
		PROGRAMMER=MEGA
	endif
	ifeq (${COM},)
		COM=/dev/ttyACM0
	endif
# Add other targets here
endif
endif
endif
endif

# set F_CPU if config name includes it (eg 8MHz, 16MHz)
ifeq ($(findstring 16MHz,${CONF}),16MHz)
    F_CPU=16000000L
else
ifeq ($(findstring 8MHz,${CONF}),8MHz)
    F_CPU=8000000L
else
ifeq ($(findstring 20MHz,${CONF}),20MHz)
    F_CPU=20000000L
endif
endif
endif

# Set upload options
ifeq (${PROGRAMMER},)
	PROGRAMMER=UNO
endif
AVRDUDE_OPTIONS=-p ${MCU}
ifeq (${PROGRAMMER},ISP)
        AVRDUDE_OPTIONS+= -c arduinoisp 
endif
ifeq (${PROGRAMMER},SHIELD)
        AVRDUDE_OPTIONS+= -c stk500v1 -b 19200 -P ${COM}
endif
ifeq (${PROGRAMMER},UNO)
        AVRDUDE_OPTIONS+= -c arduino -b 115200 -P ${COM}
endif
ifeq (${PROGRAMMER},MEGA)
        AVRDUDE_OPTIONS+= -c wiring -b 115200 -P ${COM}
endif
# Add options for other programmers here if needed

# Special targets to be called from project's specific Makefile

.build-exe:
	avr-objcopy -O ihex ${CND_ARTIFACT_PATH_${CONF}} ${CND_ARTIFACT_PATH_${CONF}}.hex
	avr-nm --synthetic -S -C --size-sort ${CND_ARTIFACT_PATH_${CONF}} >${CND_ARTIFACT_PATH_${CONF}}.nm.txt
	avr-objdump -m ${ARCH} -x -d -C ${CND_ARTIFACT_PATH_${CONF}} >${CND_ARTIFACT_PATH_${CONF}}.dump.txt
	avr-size -C --mcu=${MCU} ${CND_ARTIFACT_PATH_${CONF}}

upload:
	avrdude ${AVRDUDE_OPTIONS} -Uflash:w:${CND_ARTIFACT_PATH_${CONF}}.hex:i 

flash: upload

#TODO target to upload fuses (TODO: define default fuses for each configuration)
