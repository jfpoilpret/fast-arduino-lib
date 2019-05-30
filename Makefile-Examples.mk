#   Copyright 2016-2019 Jean-Francois Poilpret
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

# Makefile defining all FastArduino examples to build in various configurations (targets)

# Examples common to all targets (except ATtinyX5)
COMMON_EXAMPLES=	i2c/AccelGyro1 i2c/AccelGyro2											\
					analog/AnalogPin1 analog/AnalogPin2										\
					i2c/DS1307RTC1 i2c/DS1307RTC2											\
					eeprom/Eeprom1 eeprom/Eeprom2 eeprom/Eeprom3 eeprom/Eeprom4				\
					events/EventApp1 events/EventApp2 events/EventApp3 events/EventApp4		\
					events/EventApp6														\
					int/ExternalInterrupt1 int/ExternalInterrupt2							\
					io/FastPin1 io/FastPin2 io/FastPin3 io/FastPin4 io/FastPin5				\
					misc/Flash1	misc/IOStreams2												\
					i2c/Magneto1															\
					i2c/MultiIO1 i2c/MultiIO2												\
					pci/PinChangeInterrupt1 pci/PinChangeInterrupt2 pci/PinChangeInterrupt3	\
					analog/AllPWM															\
					analog/PWM1 analog/PWM2 analog/PWM3 analog/PWM4							\
					rtt/RTTApp1b rtt/RTTApp2 rtt/RTTApp3 rtt/RTTApp4						\
					motors/Servo1 motors/Servo2												\
					rtt/TimerApp3 rtt/TimerApp4												\
					spi/RF24App1 spi/RF24App2 spi/WinBond									\
					uart/UartApp2 uart/UartApp3 uart/UartApp4								\
					sonar/Sonar1 sonar/Sonar2 sonar/Sonar3 sonar/Sonar4						\
					sonar/Sonar5 sonar/Sonar6 sonar/Sonar7

EXAMPLES_ARDUINO_UNO=	complete/Conway							\
						events/EventApp5						\
						int/ExternalInterrupt3					\
						misc/IOStreams3							\
						misc/QueueCheck							\
						pci/PinChangeInterrupt4					\
						rtt/InputCapture1						\
						sonar/Sonar10 sonar/Sonar11				\
						i2c/MultiIO0 i2c/MultiIO3 i2c/MultiIO4	\
						tones/tones1 tones/tones2 tones/tones3	\
						uart/UartApp1							\
						uart/UartApp5							\
						uart/UartApp6							

EXAMPLES_ARDUINO_LEONARDO=	complete/Conway						\
							int/ExternalInterrupt3				\
							rtt/InputCapture1					\
							uart/UartApp1						

EXAMPLES_ARDUINO_MEGA=	int/ExternalInterrupt3					\
						pci/PinChangeInterrupt4					\
						rtt/InputCapture1						\
						i2c/MultiIO0							\
						uart/UartApp1							\
						uart/UartApp5							\
						uart/UartApp6							

EXAMPLES_ARDUINO_NANO=	int/ExternalInterrupt3					\
						events/EventApp5						\
						pci/PinChangeInterrupt4					\
						rtt/InputCapture1						\
						i2c/MultiIO0 i2c/MultiIO3 i2c/MultiIO4	\
						sonar/Sonar10 sonar/Sonar11				\
						uart/UartApp1							\
						uart/UartApp5							\
						uart/UartApp6							

EXAMPLES_BREADBOARD_ATMEGA328P=	int/ExternalInterrupt3					\
								events/EventApp5						\
								pci/PinChangeInterrupt4					\
								rtt/InputCapture1						\
								i2c/MultiIO0 i2c/MultiIO3 i2c/MultiIO4	\
								sonar/Sonar10 sonar/Sonar11				\
								uart/UartApp1							\
								uart/UartApp5							\
								uart/UartApp6					

EXAMPLES_BREADBOARD_ATTINYX4=	complete/Conway					\
								misc/IOStreams1					\
								pci/PinChangeInterrupt4			\
								rtt/InputCapture1

# ATtinyX5 needs its own (reduced) set of examples (because of many limitations)
EXAMPLES_BREADBOARD_ATTINYX5=	io/FastPin1 io/FastPin2			\
								uart/UartApp2					\
								analog/AnalogPin1				\
								analog/AllPWM					\
								analog/PWM1 analog/PWM2			\
								analog/PWM4						\
								eeprom/Eeprom1					\
								events/EventApp6				\
								int/ExternalInterrupt1			\
								int/ExternalInterrupt2			\
								misc/IOStreams2					\
								pci/PinChangeInterrupt1			\
								rtt/RTTApp1b rtt/RTTApp3		\
								rtt/RTTApp4						\
								rtt/TimerApp3 rtt/TimerApp4		\
								rtt/TimerTinyX5					\
								i2c/DS1307RTC1 i2c/DS1307RTC2	\
								i2c/MultiIO1 i2c/MultiIO2		\
								spi/RF24App1 spi/WinBond

# Finally define all examples supported for the current variant (defined by current configuration)
# Note that ATtinyX5 needs its own (reduced) set of examples (because of many limitations)
ifeq ($(VARIANT), BREADBOARD_ATTINYX5)
	ALL_EXAMPLES = ${EXAMPLES_${VARIANT}}
else
	ALL_EXAMPLES = ${COMMON_EXAMPLES} ${EXAMPLES_${VARIANT}}
endif

REALLY_ALL_EXAMPLES =	${COMMON_EXAMPLES} ${EXAMPLES_ARDUINO_UNO} ${EXAMPLES_ARDUINO_LEONARDO} \
						${EXAMPLES_ARDUINO_MEGA} ${EXAMPLES_ARDUINO_NANO} ${EXAMPLES_BREADBOARD_ATMEGA328P} \
						${EXAMPLES_BREADBOARD_ATTINYX4} ${EXAMPLES_BREADBOARD_ATTINYX5}

# Special build target for all fastArduino examples
# Need to export all necessary variables to submakes
export CONF PROGRAMMER COM
export VARIANT MCU ARCH F_CPU
export DUDE_OPTION CAN_PROGRAM_EEPROM CAN_PROGRAM_FUSES DUDE_SERIAL DUDE_SERIAL_RESET

examples: build
	$(foreach example, $(ALL_EXAMPLES), $(MAKE) -C examples/$(example);)

clean-examples: clean
	$(foreach example, $(ALL_EXAMPLES), $(MAKE) -C examples/$(example) clean;)

clean-examples-all: clean-all
	$(foreach example, $(REALLY_ALL_EXAMPLES), $(MAKE) -C examples/$(example) clean-all;)

