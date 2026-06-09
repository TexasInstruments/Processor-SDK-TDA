########################################################################

ifeq ($(BUILD_CPU_C7x_1),yes)
ifeq ($(TARGET_CPU),C7604)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=c7x_1

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_c7x_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_2),yes)
ifeq ($(TARGET_CPU),C7604)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=c7x_2

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_c7x_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_3),yes)
ifeq ($(TARGET_CPU),C7604)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=c7x_3

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_c7x_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_4),yes)
ifeq ($(TARGET_CPU),C7604)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=c7x_4

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_c7x_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_DMCU0),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=dmcu0

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU0),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu0

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

ifeq ($(BUILD_ENABLE_ETHFW),yes)
DEFS+=ENABLE_ETHFW
endif

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU1),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu1

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU2),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu2

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU3),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu3

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU4),yes)
ifeq ($(TARGET_CPU),M55)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=mcu4

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_m55_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU0_0),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu0_0

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU0_1),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu0_1

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU1_0),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu1_0

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU1_1),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu1_1

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU2_0),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu2_0

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################

ifeq ($(BUILD_CPU_RMCU2_1),yes)
ifeq ($(TARGET_CPU),R52P)

# CPU_ID must be set before include $(PRELUDE)
CPU_ID=rmcu2_1

_MODULE=$(CPU_ID)
include $(PRELUDE)

TARGET      := app_rtos_common_$(CPU_ID)
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

DEFS+=APP_CFG_FILE=\"app_cfg_$(CPU_ID).h\"
DEFS+=CPU_$(CPU_ID)

IDIRS+=$(VISION_APPS_PATH)/platform/$(SOC)/rtos

include $(VISION_APPS_PATH)/platform/$(SOC)/rtos/concerto_r52p_inc.mak

include $(FINALE)

endif
endif

########################################################################
