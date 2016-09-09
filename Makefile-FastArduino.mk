# Generic Makefile for FastArduino library, FastArduino examples and any FastArduino-based program.
# To use you must include it in your project Makefile by adding the following line (without the '#' comment prefix):
#
# include <path-tofastarduino>/Makefile-FastArduino.mk
#
# TODO More descriptions here
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
		PROGRAMMER=/dev/ttyACM0
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
		PROGRAMMER=/dev/ttyACM0
	endif
# Add other targets here
endif
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

#TODO target to upload fuses (TODO: define default fuses for each configuration)
