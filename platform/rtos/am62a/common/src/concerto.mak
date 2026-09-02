
########################################################################
ifeq ($(BUILD_CPU_C7x_1),yes)
  ifeq ($(TARGET_CPU),C7524)

    CPU_ID=c7x_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := app_rtos_common_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    DEFS+=CORE_CFG_FILE="core_cfg.h"
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

    DEFS+=CORE_CFG_FILE="core_cfg.h"
    DEFS+=CPU_$(CPU_ID)

    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores/$(CPU_ID)/include
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/common/include
    IDIRS+=$(PLATFORM_PATH)

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif
