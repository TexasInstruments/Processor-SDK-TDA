
########################################################################
ifeq ($(BUILD_CPU_C7x_1),yes)
  ifeq ($(TARGET_CPU),C7120)

    CPU_ID=c7x_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_C7x_2),yes)
  ifeq ($(TARGET_CPU),C7120)

    CPU_ID=c7x_2

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_C7x_3),yes)
  ifeq ($(TARGET_CPU),C7120)

    CPU_ID=c7x_3

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU1_0),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu1_0

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU2_0),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu2_0

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)
    ifeq ($(BUILD_MCU_BOARD_DEPENDENCIES),yes)
      DEFS+=BUILD_MCU_BOARD_DEPENDENCIES
    endif

    ifeq ($(BUILD_ENABLE_ETHFW),yes)
      DEFS+=ENABLE_ETHFW
    endif

    # Touch source to avoid stale objects when switching OS builds
    $(shell touch $(PLATFORM_PATH)/rtos/$(SOC)/common/src/app_init.c)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU2_1),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu2_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU3_0),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu3_0

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU3_1),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu3_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU4_0),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu4_0

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
########################################################################
ifeq ($(BUILD_CPU_MCU4_1),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu4_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE=\"core_cfg.h\"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
