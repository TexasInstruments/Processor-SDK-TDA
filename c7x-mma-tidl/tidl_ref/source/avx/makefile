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
ALGBASE_PATH ?= $(abspath ../../../)
PSDK_INSTALL_PATH ?= $(abspath ../../../../)
include $(ALGBASE_PATH)/makerules/config.mk
include $(ALGBASE_PATH)/makerules/rules.mk

##############################################################

SUBDIRS = .

CFILES= $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
HFILES = $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))

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
OUTFILE=$(LIBDIR)/tidl_avx_kernels.lib
else
OUTFILE=$(LIBDIR)/libtidl_avx_kernels.a
endif
else
OUTFILE=$(LIBDIR)/tidl_avx_kernels.lib
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

ifeq ($(TARGET_PLATFORM) , PC)
    ifdef SystemRoot
        CFLAGS += /arch:AVX2
    else
        CFLAGS += -mavx -mavx2  -mfma
    endif
endif

##############################################################

##############################################################
$(OUTFILE) : libfile

##############################################################

