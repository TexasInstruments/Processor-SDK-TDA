ifneq ($(TARGET_PLATFORM),PC)
	ifeq ($(TARGET_OS),$(filter $(TARGET_OS),SYSBIOS FREERTOS SAFERTOS QNX THREADX))

        include $(PRELUDE)
        TARGET     := app_utils_udma
        TARGETTYPE := library

        CSOURCES   := app_udma.c

        ifeq ($(SOC),$(filter $(SOC),j721e j721s2 j784s4 j742s2 j722s))
            CSOURCES += app_udma_utils.c
            CSOURCES += app_udma_test.c
        endif

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C71 C7120 C7504 C7524 C7604))
            CFLAGS   += -DBUILD_C7X
        endif

        ifeq ($(TARGET_CPU),$(filter $(TARGET_CPU),C66))
            CFLAGS   += -DBUILD_C66X
        endif

        ifeq ($(RTOS_SDK),pdk)
            IDIRS    += $(PDK_PATH)/packages/ti/drv/udma
        endif

        ifeq ($(RTOS_SDK),mcu_sdk)
            IDIRS    += $(DMA_UTILS_PATH)
            IDIRS    += $(DMA_UTILS_PATH)/udma_standalone
            IDIRS    += $(DMA_UTILS_PATH)/udma_standalone/include
            IDIRS    += $(DMA_UTILS_PATH)/csl
            IDIRS    += $(DMA_UTILS_PATH)/include
            IDIRS    += $(MCU_SDK_PATH)/source/drivers/Udma/v0/soc/$(SOC)
            IDIRS    += $(MCU_SDK_PATH)/source/hal/Bcdma/v0/include
            IDIRS    += $(MCU_SDK_PATH)/source/hal/Dru/v0/include
            IDIRS    += $(MCU_SDK_PATH)/source/hal/Psilcfg/v0/include
        endif

        ifeq ($(TARGET_OS),QNX)
            SKIPBUILD=1
        endif

        ifeq ($(TISDK_IMAGE),edgeai)
            ifeq ($(SOC),am62a)
                ifeq ($(TARGET_OS),QNX)
                    TARGET      := app_utils_udma
                    TARGETTYPE  := library
                    CSOURCES    := app_udma.c
                    IDIRS       += $(PDK_QNX_PATH)/packages/ti/drv/udma
                    IDIRS       += $(PDK_QNX_PATH)/packages/ti/drv/udma/soc/$(SOC)
                endif
            endif
        endif

        include $(FINALE)
    endif
endif
