#
# module name       :TIDL
#
# module descripton :TI Deep learning Library module is TIs CNN/DNN
#                    acceleration on EVE and DSP
#
# Copyright (C) 2016-2017 Texas Instruments Incorporated - http://www.ti.com/
#
# ALL RIGHTS RESERVED
#
#
#
ALGBASE_PATH ?= $(abspath ../../)
PSDK_INSTALL_PATH ?= $(abspath ../../../)
include $(ALGBASE_PATH)/makerules/config.mk

##############################################################
DSP_COMMON_DIR= ../../common

##############################################################

SUBDIRS = ./src

CFILES =
HFILES =
CPPFILES =

##############################################################
INCLUDE_PATHS+= -I ../tidl_ref/include
##############################################################
#Including the stats collection .h file folder
##############################################################
INCLUDE_PATHS+= -I ./inc/stats
##############################################################

########################################################################
#Stats collection source files will be compiled only in HOST_EMULATION mode
########################################################################
ifeq ($(TARGET_PLATFORM), PC)
SUBDIRS += ./src/stats
HFILES += ../common/inc/tidl_activation_optimizations.h
endif
########################################################################

CFILES += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
HFILES += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))
ifeq ($(TIDL_ENABLE_OPERATOR_WITH_UNIMPLEMENTED_KERNELS),1)
CFLAGS += -DTIDL_KERNEL_NOT_IMPLEMENTED
else
CFILES := $(filter-out ./src/tidl_rmsNorm.c,$(CFILES))
HFILES := $(filter-out ./inc/tidl_rmsNorm.h,$(HFILES))
endif
CPPFILES += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp))

########################################################################
#Common files is getting executed in Target and other modules
########################################################################
INCLUDE_PATHS+= -I ../common/inc
HFILES += ../common/inc/tidl_common_utils_infer_import.h
CFILES += ../common/src/tidl_common_utils_infer_import.c
HFILES += ../common/inc/tidl_shapeInference.h
CFILES += ../common/src/tidl_shapeInference.c

########################################################################
#Stats Collection file is getting executed in Target for the following layers
########################################################################
HFILES += ./inc/tidl_traceUtils.h
CFILES += ./src/tidl_traceUtils.c
########################################################################
CFILES+= $(ALGBASE_PATH)/ti_dl/utils/perfsim/gc_helper.c

CMD_FILE= tidl_alg.cmd

LIB_DIR=algo

ifeq ($(TIDL_BUILD_PATHS), LEGACY)
ifeq ($(TARGET_PLATFORM) , PC)
LIBDIR  = ../lib/PC/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
else
LIBDIR  = ../lib/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
endif
else
ifeq ($(TARGET_PLATFORM) , PC)
LIBDIR  = ../lib/$(TARGET_SOC)/PC/$(LIB_DIR)/$(TARGET_BUILD)
else
LIBDIR  = ../lib/$(TARGET_SOC)/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
endif
endif

ifeq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
OUTFILE=$(LIBDIR)/tidl_algo.lib
else
OUTFILE=$(LIBDIR)/libtidl_algo.a
endif
else
OUTFILE=$(LIBDIR)/tidl_algo.lib
endif

ifeq ($(CODE_COVERAGE_ENABLED_FOR_TIDL),yes)
APP_UTILS_PATH?=$(PSDK_INSTALL_PATH)/app_utils
LDIRS += $(APP_UTILS_PATH)/lib/$(TARGET_SOC)/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_file_io
STATIC_LIBS += app_utils_console_io
STATIC_LIBS += app_utils_rtos
INCLUDE_PATHS+= -I $(APP_UTILS_PATH)/utils/file_io/include/
INCLUDE_PATHS+= -I $(APP_UTILS_PATH)/utils/console_io/include/
INCLUDE_PATHS+= -I $(APP_UTILS_PATH)/utils/timer/include/
endif
##############################################################


##############################################################
include $(ALGBASE_PATH)/makerules/rules.mk
include $(ALGBASE_PATH)/makerules/common.mk
#used inside makerules, but okay to define it afterwards
INCLUDE_PATHS+= -I .
INCLUDE_PATHS+= -I ./inc
INCLUDE_PATHS+= -I ./inc/dummy
INCLUDE_PATHS+= -I ../inc
INCLUDE_PATHS+= -I $(DSP_COMMON_DIR)
INCLUDE_PATHS+= -I ../../tidl_ref/include
INCLUDE_PATHS+= -I ../../tidl-kernels/include
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
INCLUDE_PATHS+= -I ../../tidl-kernels/utils/floating_point/include
endif
INCLUDE_PATHS+= -I ../../tidl-kernels/source
INCLUDE_PATHS+= -I ../../tidl_priv/include
INCLUDE_PATHS+= -I ../../tidl_priv/source/tidsp
ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a J722S j722s TDA54 tda54))
INCLUDE_PATHS+= -I ../../tidl_priv/source
endif
INCLUDE_PATHS+= -I $(ALGBASE_PATH)/ti_dl/utils/perfsim

# For HOST_EMULATION (PC) builds: tidl_shapeInference.c includes ti_dl.h
# and tidl_import_config.h under #ifdef HOST_EMULATION.
ifeq ($(TARGET_PLATFORM), PC)
INCLUDE_PATHS+= -I ../utils/tidlModelImport
endif

#mv7100 should be in rules.mk
CFLAGS += $(INCLUDE_PATHS)


##############################################################


##############################################################
LDFLAGS= -l "./tidl_alg.cmd"
ifeq ($(CORE) , eve)
$(OUTFILE) : libfile
else ifeq ($(CORE) , dsp)
$(OUTFILE) : libfile
endif

##############################################################

