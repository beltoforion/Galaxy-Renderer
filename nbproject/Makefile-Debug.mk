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
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CumulativeDistributionFunction.o \
	${OBJECTDIR}/FastMath.o \
	${OBJECTDIR}/Galaxy.o \
	${OBJECTDIR}/GalaxyProp.o \
	${OBJECTDIR}/NBodyWnd.o \
	${OBJECTDIR}/OrbitCalculator.o \
	${OBJECTDIR}/SDLWnd.o \
	${OBJECTDIR}/Star.o \
	${OBJECTDIR}/Types.o \
	${OBJECTDIR}/Vector.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/specrend.o


# C Compiler Flags
CFLAGS=-Wall -O3 -std=c++11

# CC Compiler Flags
CCFLAGS=-std=c++11
CXXFLAGS=-std=c++11

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=`sdl-config --libs` `pkg-config --libs x11` `pkg-config --libs glu` `pkg-config --libs gl`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/galaxy-renderer

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/galaxy-renderer: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/galaxy-renderer ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/CumulativeDistributionFunction.o: CumulativeDistributionFunction.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CumulativeDistributionFunction.o CumulativeDistributionFunction.cpp

${OBJECTDIR}/FastMath.o: FastMath.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FastMath.o FastMath.cpp

${OBJECTDIR}/Galaxy.o: Galaxy.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Galaxy.o Galaxy.cpp

${OBJECTDIR}/GalaxyProp.o: GalaxyProp.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GalaxyProp.o GalaxyProp.cpp

${OBJECTDIR}/NBodyWnd.o: NBodyWnd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/NBodyWnd.o NBodyWnd.cpp

${OBJECTDIR}/OrbitCalculator.o: OrbitCalculator.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OrbitCalculator.o OrbitCalculator.cpp

${OBJECTDIR}/SDLWnd.o: SDLWnd.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/SDLWnd.o SDLWnd.cpp

${OBJECTDIR}/Star.o: Star.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Star.o Star.cpp

${OBJECTDIR}/Types.o: Types.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Types.o Types.cpp

${OBJECTDIR}/Vector.o: Vector.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Vector.o Vector.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/specrend.o: specrend.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g `sdl-config --cflags` `pkg-config --cflags x11` `pkg-config --cflags glu` `pkg-config --cflags gl`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/specrend.o specrend.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/galaxy-renderer

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
