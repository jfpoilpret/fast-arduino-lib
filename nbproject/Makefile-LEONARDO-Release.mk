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
CND_CONF=LEONARDO-Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/cores/fastarduino/abi.o \
	${OBJECTDIR}/cores/fastarduino/boards/common_traits.o \
	${OBJECTDIR}/cores/fastarduino/events.o \
	${OBJECTDIR}/cores/fastarduino/linked_list.o \
	${OBJECTDIR}/cores/fastarduino/main.o \
	${OBJECTDIR}/cores/fastarduino/power.o \
	${OBJECTDIR}/cores/fastarduino/soft_uart.o \
	${OBJECTDIR}/cores/fastarduino/spi.o \
	${OBJECTDIR}/cores/fastarduino/streams.o \
	${OBJECTDIR}/cores/fastarduino/time.o \
	${OBJECTDIR}/cores/fastarduino/watchdog.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -DNO_ABI -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues
CXXFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -DNO_ABI -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues

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

${OBJECTDIR}/cores/fastarduino/abi.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/abi.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/abi.o cores/fastarduino/abi.cpp

${OBJECTDIR}/cores/fastarduino/boards/common_traits.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/boards/common_traits.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino/boards
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/boards/common_traits.o cores/fastarduino/boards/common_traits.cpp

${OBJECTDIR}/cores/fastarduino/events.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/events.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/events.o cores/fastarduino/events.cpp

${OBJECTDIR}/cores/fastarduino/linked_list.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/linked_list.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/linked_list.o cores/fastarduino/linked_list.cpp

${OBJECTDIR}/cores/fastarduino/main.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/main.o cores/fastarduino/main.cpp

${OBJECTDIR}/cores/fastarduino/power.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/power.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/power.o cores/fastarduino/power.cpp

${OBJECTDIR}/cores/fastarduino/soft_uart.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/soft_uart.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/soft_uart.o cores/fastarduino/soft_uart.cpp

${OBJECTDIR}/cores/fastarduino/spi.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/spi.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/spi.o cores/fastarduino/spi.cpp

${OBJECTDIR}/cores/fastarduino/streams.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/streams.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/streams.o cores/fastarduino/streams.cpp

${OBJECTDIR}/cores/fastarduino/time.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/time.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/time.o cores/fastarduino/time.cpp

${OBJECTDIR}/cores/fastarduino/watchdog.o: nbproject/Makefile-${CND_CONF}.mk cores/fastarduino/watchdog.cpp 
	${MKDIR} -p ${OBJECTDIR}/cores/fastarduino
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -Icores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/cores/fastarduino/watchdog.o cores/fastarduino/watchdog.cpp

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
