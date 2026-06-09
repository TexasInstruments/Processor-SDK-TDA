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
include $(ALGBASE_PATH)/makerules/rules.mk

##############################################################

SUBDIRS = .

##############################################################
#Including the reference .h file folder
##############################################################
INCLUDE_PATHS+= -I ../include/ref
##############################################################
CFILES =
HFILES =
########################################################################
#Reference source files will be compiled only in HOST_EMULATION mode
########################################################################
ifeq ($(TARGET_PLATFORM), PC)
CFILES += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
HFILES += $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))
endif

########################################################################
#Build CUDA files if enabled
########################################################################
ifeq ($(BUILD_WITH_CUDA),yes)
CFILES+= $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/cuda/*.cu))
endif

########################################################################
#Reference file is getting executed in Target for the following layers
########################################################################
HFILES += ../include/tidl_innerProduct_ref.h
CFILES += tidl_innerProduct_ref.c

HFILES += ../include/tidl_resize_ref.h
CFILES += tidl_resize_ref.c

HFILES += ../include/tidl_softmax_ref.h
CFILES += tidl_softmax_ref.c
########################################################################

LIB_DIR=algo

ifeq ($(TIDL_BUILD_PATHS), LEGACY)
ifeq ($(TARGET_PLATFORM) , PC)
LIBDIR  = $(ALGBASE_PATH)/ti_dl/lib/PC/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
else
LIBDIR  = $(ALGBASE_PATH)/ti_dl/lib/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
endif
else
ifeq ($(TARGET_PLATFORM) , PC)
LIBDIR  = $(ALGBASE_PATH)/ti_dl/lib/$(TARGET_SOC)/PC/$(LIB_DIR)/$(TARGET_BUILD)
else
LIBDIR  = $(ALGBASE_PATH)/ti_dl/lib/$(TARGET_SOC)/$(CORE)/$(LIB_DIR)/$(TARGET_BUILD)
endif
endif

ifeq ($(TARGET_PLATFORM) , PC)
ifdef SystemRoot
OUTFILE=$(LIBDIR)/tidl_ref.lib
else
OUTFILE=$(LIBDIR)/libtidl_ref.a
endif
else
OUTFILE=$(LIBDIR)/tidl_ref.lib
endif

##############################################################


##############################################################
include $(ALGBASE_PATH)/makerules/rules.mk
include $(ALGBASE_PATH)/makerules/common.mk
#used inside makerules, but okay to define it afterwards
INCLUDE_PATHS+= -I ../include
INCLUDE_PATHS+= -I $(ALGBASE_PATH)/ti_dl/inc
INCLUDE_PATHS+= -I $(ALGBASE_PATH)/ti_dl/algo/inc
INCLUDE_PATHS+= -I $(ALGBASE_PATH)/ti_dl/common/inc
INCLUDE_PATHS+= -I $(ALGBASE_PATH)/ti_dl/utils/perfsim

CFLAGS += $(INCLUDE_PATHS)

##############################################################
ifeq ($(BUILD_WITH_OPENACC), yes)
	CC = nvc++ -c -fast -acc -Minfo=accel -gpu=ccall -tp=px
endif

##############################################################
$(OUTFILE) : libfile

##############################################################

