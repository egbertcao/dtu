#------------------------------------------------------------
# (C) Copyright [2006-2008] Marvell International Ltd.
# All Rights Reserved
#------------------------------------------------------------

#=========================================================================
# File Name      : MainApp.mak
# Description    : Main make file for the thirdparty/thirdparty group.
#
# Usage          : make [-s] -f thirdparty.mak OPT_FILE=<path>/<opt_file>
#
# Notes          : The options file defines macro values defined
#                  by the environment, target, and groups. It
#                  must be included for proper group building.
#
#=========================================================================

# Group build options
include ${OPT_FILE}

# Group Makefile information
GEN_GROUP_MAKEFILE = ${BUILD_ROOT}/env/${HOST}/build/group.mak

# Define Group ---------------------------------------

GROUP_NAME     = MainApp
GROUP_BASE     = custom
GROUP_DEP_FILE = MainApp_dep.mak
GROUP_PATH = ${BUILD_ROOT}/$(GROUP_BASE)/$(GROUP_NAME)

# The relative path locations of local source and include file directories.
LOCAL_SRC_PATH    = $(GROUP_PATH)/src
LOCAL_INC_PATHS   = $(GROUP_PATH)/src $(GROUP_PATH)/inc \
		${BUILD_ROOT}\os\nu_xscale\inc    \
		${BUILD_ROOT}\os\osa\inc \
		${BUILD_ROOT}\pcac\openssl\inc 
		
ifneq (,$(findstring OPENCPU,${VARIANT_LIST}))
LOCAL_INC_PATHS   += ${BUILD_ROOT}\include\hal\oc_include 
else
LOCAL_INC_PATHS   += ${BUILD_ROOT}\hal\oc_include 
endif		       


# Define Default Group Variant -----------------------

# Group source files, paths not required
LOCAL_SRC_FILES = customer_MainApp.c


# local group build flags for source files
# contained in this group directory
LOCAL_CFLAGS  =
LOCAL_DFLAGS  =

# These are the tool flags specific to the customer_app group only.
# The environment, target, and group also set flags.
GROUP_CFLAGS  =
GROUP_DFLAGS  =
GROUP_ARFLAGS =

GROUP_PACKAGE_LIST = custom\gpio \
										 custom\adc \
										 custom\eint \
										 custom\i2c \
										 custom\spi \
										 custom\uart \
										 custom\sleep \
										 custom\WatchDog \
										 custom\pwm \
										 custom\fs \
										 custom\ftp \
										 custom\socket \
										 custom\net \
										 custom\alimqtt \
										 custom\tts \
										 custom\alimmq \
										 custom\https \
										 custom\location \
										 custom\fota \
										 custom\audio \
										 custom\sms \
										 custom\at \
										 custom\poweroffAlarm \
										 custom\mqtt \
										 custom\allystar \
										 custom\ssl \
										 custom\modbus \
										 custom\dtu \
										 
GROUP_GROUP_LIST   =
# package and group variants required for this target variant
# syntax is <PACKAGE>_<VARIANT> or <GROUP>_<VARIANT>.
GROUP_VARIANT_LIST =
# Define Group Variants -------------------------------


#TV depending: dlm, TickManager; Monitor; transport; DataCollector


#========================================================================

GROUP_GROUP_LIST   =

#========================================================================
#          PACKAGES
#========================================================================
    GROUP_DFLAGS += -DDATALIGHT

# GROUP_TRACE_LIST contains package and groups in which diagnostics/tracing
# will be compiled. Not compiling diagnostics/tracing in some modules
# will reduce code size and increase code performance.
GROUP_TRACE_LIST   =  $(GROUP_PACKAGE_LIST)

GROUP_TRACE_TYPE   = STRING_TRACING

# Include the Standard Group Make File ---------------
include ${GEN_GROUP_MAKEFILE}

# Include the Make Dependency File ---------------------
# This must be the last line in the file
include ${GROUP_DEP_FILE}
