#------------------------------------------------------------
# (C) Copyright [2006-2008] Marvell International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : WatchDog.mak
# Description    : Main make file for the hal/GPIO package.
#
# Usage          : make [-s] -f WatchDog.mak OPT_FILE=<path>/<opt_file>
#
# Notes          : The options file defines macro values defined
#                  by the environment, target, and groups. It
#                  must be included for proper package building.
#
# Copyright (c) 2001 Intel Corporation. All Rights Reserved
#=========================================================================

# Package build options
include ${OPT_FILE}

# Package Makefile information
GEN_PACK_MAKEFILE = ${BUILD_ROOT}/env/${HOST}/build/package.mak

# Define Package ---------------------------------------

PACKAGE_NAME     = WatchDog
PACKAGE_BASE     = custom
PACKAGE_DEP_FILE = WatchDog_dep.mak
PACKAGE_PATH     = ${BUILD_ROOT}/${PACKAGE_BASE}/${PACKAGE_NAME}

# The relative path locations of source and include file directories.
PACKAGE_SRC_PATH    = ${PACKAGE_PATH}/src
PACKAGE_INC_PATHS   = ${PACKAGE_PATH}/src ${PACKAGE_PATH}/inc 

ifneq (,$(findstring OPENCPU,${VARIANT_LIST}))
PACKAGE_INC_PATHS   += ${BUILD_ROOT}\include\hal\oc_include 
else
PACKAGE_INC_PATHS   += ${BUILD_ROOT}\hal\oc_include 
endif 

PACKAGE_SRC_FILES = demo_watchdog.c
		
 
# These are the tool flags specific to the GPIO package only.
# The environment, target, and group also set flags.
PACKAGE_CFLAGS  =
PACKAGE_DFLAGS  =

PACKAGE_ARFLAGS =

# Define Package Variants -------------------------------

# Include the Standard Package Make File ---------------
include ${GEN_PACK_MAKEFILE}

# Include the Make Dependency File ---------------------
# This must be the last line in the file
include ${PACKAGE_DEP_FILE}
