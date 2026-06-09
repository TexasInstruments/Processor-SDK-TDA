# Top-level makefile for ctools library (cpt2, gtc, tbr, atbrep, trace_aggr)
ifeq ($(CTOOLS_BUILD_ENABLED), yes)
ifeq ($(TARGET_PLATFORM),$(filter $(TARGET_PLATFORM), J722S J721E J784S4 J721S2 J742S2))
ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU), A72 A53))
ifeq ($(TARGET_OS),$(filter $(TARGET_OS), LINUX QNX))
include $(PRELUDE)

TARGET      := app_utils_ctools
TARGETTYPE  := library

# Source files for all ctools drivers
CSOURCES += \
	app/src/cpt2_ddr_probe.c \
	cpt2/src/cpt2.c \
	cpt2/src/cpt2_access.c \
	gtc/src/gtc.c \
	gtc/src/gtc_access.c \
	tbr/src/tbr.c \
	tbr/src/tbr_access.c \
	atbrep/src/atbrep.c \
	atbrep/src/atbrep_access.c \
	trace_aggr/src/trace_aggr.c \
	trace_aggr/src/trace_aggr_access.c \
	common/src/td_error.c

# Include path for all driver headers
CTOOLS_SUBDIRS = atbrep common cpt2 gtc tbr trace_aggr
IDIRS += $(APP_UTILS_PATH)/utils/ctools/include
IDIRS += $(APP_UTILS_PATH)/utils/ctools/include/soc/$(SOC)
IDIRS += $(foreach dir,$(CTOOLS_SUBDIRS),$(APP_UTILS_PATH)/utils/ctools/src/$(dir)/include)

DEFS += _SELF_HOSTED
DEFS += _64_BIT_TARGET

include $(FINALE)

endif
endif
endif
endif