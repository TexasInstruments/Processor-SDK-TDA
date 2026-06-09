ifeq ($(TARGET_PLATFORM) , PC)
INCLUDE_PATHS += -I $(DSP_TOOLS)/host_emulation/include/C$(SI_VER)$(C7x_HOSTEMU_COMPILER_STRING)/
else
CFLAGS += -mv$(SI_VER)
endif

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721E j721e))
    CFLAGS += -DSOC_J721E
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J721S2 j721s2))
    CFLAGS += -DSOC_J721S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J784S4 j784s4))
    CFLAGS += -DSOC_J784S4
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J742S2 j742s2))
    CFLAGS += -DSOC_J742S2
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
    CFLAGS += -DSOC_AM62A
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
    CFLAGS += -DSOC_J722S
else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
    CFLAGS += -DSOC_TDA54
else
    #Temp change as we will be using j721e pdk
    CFLAGS += -DSOC_J721E
endif

INCLUDE_PATHS+= -I $(MMALIB_PATH)/ti/mmalib/src/cnn_c7xmma
INCLUDE_PATHS+= -I $(MMALIB_PATH)/ti/mmalib/src/linalg_c7xmma
INCLUDE_PATHS+= -I $(IVISION_PATH)
INCLUDE_PATHS+= -I $(IVISION_PATH)/ti/xdais
INCLUDE_PATHS+= -I /usr/local/include
INCLUDE_PATHS+= -I $(PDK_INSTALL_PATH)
ifeq ($(RTOS_SDK),mcu_sdk)
    INCLUDE_PATHS+= -I $(MCU_SDK_PATH)/source
    INCLUDE_PATHS+= -I $(DMA_UTILS_PATH)
    CFLAGS+= -DMCU_SDK
else ifeq ($(RTOS_SDK),mcu_plus_sdk)
    INCLUDE_PATHS+= -I $(MCU_PLUS_SDK_PATH)/source
    INCLUDE_PATHS+= -I $(DMA_UTILS_PATH)
    INCLUDE_PATHS+= -I $(DMA_UTILS_PATH)/udma_standalone
    CFLAGS+= -DMCU_PLUS_SDK
else
    INCLUDE_PATHS+= -I $(PDK_INSTALL_PATH)/ti/drv/udma/dmautils
endif

ifeq ($(TIDL_BUILD_FOR_LOKI) ,1)
    CFLAGS+= -DLOKI_BUILD
endif

ifeq ($(TARGET_PLATFORM) , PC)
    ifeq ($(BUILD_WITH_CUDA),yes)
        CFLAGS+= -DBUILD_WITH_CUDA
    endif
    ifeq ($(BUILD_WITH_OPENACC),yes)
        CFLAGS+= -DBUILD_WITH_OPENACC
    endif
else
    #CFLAGS+= -k
    CFLAGS+= -mw
endif

ifeq ($(TIDL_BUILD_FOR_QT), 1)
CFLAGS += -DTIDL_BUILD_FOR_QT
endif

ifeq ($(TARGET_PLATFORM) , PC)
CFLAGS += -D__C7X_HOSTEM__
CFLAGS += -DGLIBCXX_CHECK_MATH11_PROTO
CFLAGS += -D_FILE_OFFSET_BITS=64
else
CFLAGS += --cpp_default
endif
CFLAGS+= -D_TMS320C6600

ifeq ($(RTOS_SDK),mcu_plus_sdk)
CFLAGS+= -DMCU_PLUS_SDK
endif
ifeq ($(RTOS_SDK),mcu_sdk)
CFLAGS+= -DMCU_SDK
endif

# DSP build needs CGT, BIOS, and XDC include files
IDIRS += $(CGT7X_ROOT)/include
ifeq ($(RTOS),SYSBIOS)
	IDIRS += $(BIOS_PATH)/packages
	IDIRS += $(XDCTOOLS_PATH)/packages
endif

# library search dirs are always platform specific
LDIRS += $(CGT7X_ROOT)/lib
LDIRS += $(MMALIB_PATH)/lib/$(TARGET_C7X_VERSION)/$(TARGET_BUILD)
# path to tidl_algo and tidl_priv
ifeq ($(TIDL_BUILD_PATHS), LEGACY)
LDIRS += $($(_MODULE)_SDIR)/../../../lib/dsp/algo/$(TARGET_BUILD)
else
LDIRS += $($(_MODULE)_SDIR)/../../../lib/$(TARGET_SOC)/dsp/algo/$(TARGET_BUILD)
endif

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), am62a AM62A j722s J722S TDA54 tda54))
DEFS += DMA_UTILS_STANDALONE
endif

DMA_LIBS = .
ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_sdk mcu_plus_sdk))
    LDIRS += $(DMA_UTILS_PATH)/lib
endif

ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_plus_sdk mcu_sdk))
    ifeq ($(TARGET_PLATFORM) , PC)
        ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
            DMA_LIBS += dmautils.am62ax.c75x.ti-c7x-hostemu.$(TARGET_BUILD).lib
        else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
            DMA_LIBS += dmautils.j722s.c75ssx-0.ti-c7x-hostemu.$(TARGET_BUILD).lib
        else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
            DMA_LIBS += dmautils.tda54.c76x.ti-c7x-hostemu.$(TARGET_BUILD).lib
            DMA_LIBS += libdrivers-ti_sdk_cfg_default_c76_ti-c7000.a
        endif
    else
        ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
            DMA_LIBS += dmautils.am62ax.c75x.ti-c7000.$(TARGET_BUILD).lib
        else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
            DMA_LIBS += dmautils.j722s.c75ss0-0.ti-c7000.$(TARGET_BUILD).lib
        else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
            DMA_LIBS += dmautils.tda54.dsp.ti-c7000.$(TARGET_BUILD).lib
            DMA_LIBS += libdrivers-ti_sdk_cfg_default_c76_ti-c7000.a
        endif
    endif
else
    ifeq ($(TARGET_C7X_VERSION), C7504)
        LDIRS += $(PDK_PATH)/ti/csl/lib/$(SOC)/c75x/$(TARGET_BUILD)
    else
        LDIRS += $(PDK_PATH)/ti/csl/lib/$(SOC)/c7x/$(TARGET_BUILD)
    endif
    LDIRS += $(PDK_PATH)/ti/drv/ipc/lib/$(SOC)/c7x_$(C7X_CORE_ID)/$(TARGET_BUILD)

    ifeq ($(RTOS),SYSBIOS)
    	LDIRS += $(PDK_PATH)/ti/osal/lib/tirtos/$(SOC)/c7x/$(TARGET_BUILD)
    else
    	LDIRS += $(PDK_PATH)/ti/kernel/lib/$(SOC)/c7x_$(C7X_CORE_ID)/$(TARGET_BUILD)
        ifeq ($(TARGET_C7X_VERSION), C7504)
        	LDIRS += $(PDK_PATH)/ti/osal/lib/freertos/$(SOC)/c75x/$(TARGET_BUILD)
        else
        	LDIRS += $(PDK_PATH)/ti/osal/lib/freertos/$(SOC)/c7x/$(TARGET_BUILD)
        endif
    endif
    LDIRS += $(PDK_PATH)/ti/drv/sciclient/lib/$(SOC)/c7x_$(C7X_CORE_ID)/$(TARGET_BUILD)
    LDIRS += $(PDK_PATH)/ti/drv/udma/lib/$(SOC)/c7x_$(C7X_CORE_ID)/$(TARGET_BUILD)
    ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), am62a AM62A))
    else
        DMA_LIBS += udma.ae71
    endif
    DMA_LIBS += dmautils.ae71
    ifndef DMA_UTILS_STANDALONE
    ADDITIONAL_STATIC_LIBS += ipc.ae71
    ADDITIONAL_STATIC_LIBS += ti.osal.ae71
    ADDITIONAL_STATIC_LIBS += sciclient.ae71
    endif
endif

#Check if below are required
#LDIRS += $(PDK_PATH)/ti/drv/mailbox/lib/$(TARGET_SOC)/c7x_1/$(TARGET_BUILD)
#ADDITIONAL_STATIC_LIBS += mailbox.ae71

# External libraries

ADDITIONAL_STATIC_LIBS += $(DMA_LIBS)
ADDITIONAL_STATIC_LIBS += common_$(TARGET_C7X_VERSION).lib
ADDITIONAL_STATIC_LIBS += mmalib_$(TARGET_C7X_VERSION).lib
ADDITIONAL_STATIC_LIBS += mmalib_cn_$(TARGET_C7X_VERSION).lib

ifeq ($(RTOS),FREERTOS)
	ADDITIONAL_STATIC_LIBS += ti.kernel.freertos.ae71
endif
ADDITIONAL_STATIC_LIBS += ti.csl.ae71

# internal libraries
STATIC_LIBS += tidl_algo
STATIC_LIBS += tidl_ref
STATIC_LIBS += tidl_kernels
STATIC_LIBS += tidl_priv

# Custom Library
STATIC_LIBS += tidl_custom

ADDITIONAL_STATIC_LIBS += rts$(SI_VER)_le.lib
