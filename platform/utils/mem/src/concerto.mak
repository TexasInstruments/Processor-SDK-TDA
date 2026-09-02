########################################################################
ifeq ($(BUILD_CPU_C7x_1),yes)
  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7604 C7524 C7120))

    # CPU_ID must be set before include $(PRELUDE)
    CPU_ID=c7x_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_2),yes)
  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7604 C7524 C7120))

    # CPU_ID must be set before include $(PRELUDE)
    CPU_ID=c7x_2

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_3),yes)
  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7604 C7524 C7120))

    # CPU_ID must be set before include $(PRELUDE)
    CPU_ID=c7x_3

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_C7x_4),yes)
  ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7604 C7524 C7120))

    # CPU_ID must be set before include $(PRELUDE)
    CPU_ID=c7x_4

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c7x_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_m55_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r52p_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU1_0),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu1_0

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_MCU1_1),yes)
  ifeq ($(TARGET_CPU),R5F)

    CPU_ID=mcu1_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

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

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_r5f_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_C6x_1),yes)
  ifeq ($(TARGET_CPU),C66)

    CPU_ID=c6x_1

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c6x_inc.mak

    include $(FINALE)

  endif
endif

########################################################################

ifeq ($(BUILD_CPU_C6x_2),yes)
  ifeq ($(TARGET_CPU),C66)

    CPU_ID=c6x_2

    _MODULE=$(CPU_ID)
    include $(PRELUDE)

    TARGET      := rtos_mem_$(CPU_ID)
    TARGETTYPE  := library
    CSOURCES    := $(call all-c-files)

    IDIRS+=$(PLATFORM_PATH)/utils/mem/include
    IDIRS+=$(PLATFORM_PATH)/memory_map/$(SOC)
    IDIRS+=$(PLATFORM_PATH)/rtos/$(SOC)/cores

    DEFS+=CORE_CFG_FILE=\"$(CPU_ID)/include/core_cfg.h\"

    include $(PLATFORM_PATH)/rtos/$(SOC)/concerto_c6x_inc.mak

    include $(FINALE)

  endif
endif
