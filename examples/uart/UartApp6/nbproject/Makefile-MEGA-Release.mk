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
CND_CONF=MEGA-Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/UARTApp6.o


# C Compiler Flags
CFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -Wextra -flto -g -Os -ffunction-sections -fdata-sections

# CC Compiler Flags
CCFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues
CXXFLAGS=-mmcu=${MCU} -DF_CPU=${F_CPU} -D${VARIANT} -fno-exceptions -Wextra -flto -std=gnu++11 -felide-constructors -Os -ffunction-sections -fdata-sections -mcall-prologues

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../../dist/MEGA-Release/AVR-GNU-Toolchain-3.5.3-Linux/libfastarduino.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp6

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp6: ../../../dist/MEGA-Release/AVR-GNU-Toolchain-3.5.3-Linux/libfastarduino.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp6: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp6 ${OBJECTFILES} ${LDLIBSOPTIONS} -Os -Wl,--gc-sections -Wl,--relax -flto -mmcu=${MCU} -Wl,-Map,${CND_ARTIFACT_PATH_${CONF}}.map

${OBJECTDIR}/UARTApp6.o: UARTApp6.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -Wall -I../../../cores -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/UARTApp6.o UARTApp6.cpp

# Subprojects
.build-subprojects:
	cd ../../.. && ${MAKE}  -f Makefile CONF=MEGA-Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/uartapp6

# Subprojects
.clean-subprojects:
	cd ../../.. && ${MAKE}  -f Makefile CONF=MEGA-Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
