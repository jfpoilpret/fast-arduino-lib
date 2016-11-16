#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=avr-gcc
CCC=avr-g++
CXX=avr-g++
FC=gfortran
AS=avr-as

# Macros
CND_PLATFORM=AVR-GNU-Toolchain-3.5.3-Linux
CND_DLIB_EXT=so
CND_CONF=ATmega328-Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/cores/fastarduino/Events.o \
	${OBJECTDIR}/cores/fastarduino/FastSPI.o \
	${OBJECTDIR}/cores/fastarduino/LinkedList.o \
	${OBJECTDIR}/cores/fastarduino/NRF24L01.o \
	${OBJECTDIR}/cores/fastarduino/RTT.o \
	${OBJECTDIR}/cores/fastarduino/SPI.o \
	${OBJECTDIR}/cores/fastarduino/WinBond.o \
	${OBJECTDIR}/cores/fastarduino/main.o \
	${OBJECTDIR}/cores/fastarduino/power.o \
	${OBJECTDIR}/cores/fastarduino/softuart.o \
	${OBJECTDIR}/cores/fastarduino/streams.o \
	${OBJECTDIR}/cores/fastarduino/time.o \
	${OBJECTDIR}/cores/fastarduino/uart.o \
	${OBJECTDIR}/cores/fastarduino/watchdog.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues
CXXFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a

${OBJECTDIR}/cores/fastarduino/Events.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/Events.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/Events.o cores/fastarduino/Events.cpp

${OBJECTDIR}/cores/fastarduino/FastSPI.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/FastSPI.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/FastSPI.o cores/fastarduino/FastSPI.cpp

${OBJECTDIR}/cores/fastarduino/LinkedList.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/LinkedList.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/LinkedList.o cores/fastarduino/LinkedList.cpp

${OBJECTDIR}/cores/fastarduino/NRF24L01.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/NRF24L01.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/NRF24L01.o cores/fastarduino/NRF24L01.cpp

${OBJECTDIR}/cores/fastarduino/RTT.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/RTT.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/RTT.o cores/fastarduino/RTT.cpp

${OBJECTDIR}/cores/fastarduino/SPI.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/SPI.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/SPI.o cores/fastarduino/SPI.cpp

${OBJECTDIR}/cores/fastarduino/WinBond.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/WinBond.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/WinBond.o cores/fastarduino/WinBond.cpp

${OBJECTDIR}/cores/fastarduino/main.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/main.o cores/fastarduino/main.cpp

${OBJECTDIR}/cores/fastarduino/power.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/power.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/power.o cores/fastarduino/power.cpp

${OBJECTDIR}/cores/fastarduino/softuart.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/softuart.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/softuart.o cores/fastarduino/softuart.cpp

${OBJECTDIR}/cores/fastarduino/streams.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/streams.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/streams.o cores/fastarduino/streams.cpp

${OBJECTDIR}/cores/fastarduino/time.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/time.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/time.o cores/fastarduino/time.cpp

${OBJECTDIR}/cores/fastarduino/uart.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/uart.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/uart.o cores/fastarduino/uart.cpp

${OBJECTDIR}/cores/fastarduino/watchdog.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/watchdog.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/watchdog.o cores/fastarduino/watchdog.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libfastarduino.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
