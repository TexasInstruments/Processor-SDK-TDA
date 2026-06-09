ifeq ($(TARGET_PLATFORM), PC)

include $(PRELUDE)
TARGET      := PC_dsp_test_dl_algo_host_rt.out
TARGETTYPE  := exe
CSOURCES    := $(call all-c-files)

# host emulation needs HE includes
INCLUDE_PATHS+= -I $(DSP_TOOLS)/host_emulation/include/$(C7X_VERSION)$(C7x_HOSTEMU_COMPILER_STRING)/


# library search dirs are always platform specific
LDIRS += $(DSP_TOOLS)/host_emulation

ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a J722S j722s TDA54 tda54))
DEFS += DMA_UTILS_STANDALONE
endif

# SDK library paths (IDIRS are in concerto_common.mak)
ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_sdk mcu_plus_sdk))
    LDIRS += $(DMA_UTILS_PATH)/lib
endif
ifeq ($(RTOS_SDK),mcu_sdk)
    # Add MCU_SDK library path for system libraries
    ifeq ($(TARGET_BUILD),debug)
        LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Debug
    else
        LDIRS += $(MCU_SDK_PATH)/build/tda54/lib/Release
    endif
endif

ifeq ($(RTOS_SDK),$(filter $(RTOS_SDK), mcu_plus_sdk mcu_sdk))
    ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a))
        DMA_LIBS += dmautils.am62ax.c75x.ti-c7x-hostemu.$(TARGET_BUILD).lib
    else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), J722S j722s))
        DMA_LIBS += dmautils.j722s.c75ssx-0.ti-c7x-hostemu.$(TARGET_BUILD).lib
    else ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), TDA54 tda54))
        DMA_LIBS += dmautils.tda54.c76x.ti-c7x-hostemu.$(TARGET_BUILD).lib
        DMA_LIBS += libdrivers-ti_sdk_cfg_default_hostemu_gcc-linux.a
        DMA_LIBS += libhal-ti_sdk_cfg_default_hostemu_gcc-linux.a
        DMA_LIBS += libti_sdk_cfg_default_hostemu_gcc-linux.a
    endif
else
    LDIRS += $(PDK_PATH)/ti/csl/lib/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
    LDIRS += $(PDK_PATH)/ti/drv/udma/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
    ifeq ($(TARGET_SOC),$(filter $(TARGET_SOC), AM62A am62a J722S j722s TDA54 tda54))
    else
        LDIRS += $(PDK_PATH)/ti/osal/lib/nonos/$(SOC)/c7x-hostemu/$(TARGET_BUILD)
        LDIRS += $(PDK_PATH)/ti/drv/sciclient/lib/$(SOC)_hostemu/c7x-hostemu/$(TARGET_BUILD)
        ifndef DMA_UTILS_STANDALONE
        DMA_LIBS += ti.osal.lib
        DMA_LIBS += udma.lib
        DMA_LIBS += sciclient.lib
        endif
    endif

    # External libraries: The order in which they are defined is important
    DMA_LIBS  += dmautils.lib
    DMA_LIBS += ti.csl.lib
    ifeq ($(ENABLE_SDK_9_2_COMPATIBILITY), 1)
    else
      DMA_LIBS += ti.csl.init.lib
    endif
endif

LDIRS += $(MMALIB_PATH)/lib/$(C7X_VERSION)/$(TARGET_BUILD)

# path to tidl_algo and tidl_priv
ifeq ($(TIDL_BUILD_PATHS), LEGACY)
LDIRS += $(TIDL_PATH)/ti_dl/lib/PC/dsp/algo/$(TARGET_BUILD)
else
LDIRS += $(TIDL_PATH)/ti_dl/lib/$(TARGET_SOC)/PC/algo/$(TARGET_BUILD)
endif

ADDITIONAL_STATIC_LIBS += $(DMA_LIBS)
# internal libraries
STATIC_LIBS += tidl_algo
STATIC_LIBS += tidl_ref
STATIC_LIBS += tidl_kernels
STATIC_LIBS += tidl_priv

# Custom Library
ifeq ($(TIDL_TEST_CUSTOM_LAYER), 1)
DEFS += TIDL_TEST_CUSTOM_LAYER
STATIC_LIBS += tidl_custom
endif

# External libraries: The order in which they are defined ins important
# Also, must be defined after the internal libraries
#STATIC_LIBS += mmalib_cn_x86_64
#STATIC_LIBS += mmalib_x86_64
#STATIC_LIBS += common_x86_64
STATIC_LIBS += $(C7X_VERSION)$(C7x_HOSTEMU_COMPILER_STRING)-host-emulation


# defines for host emulation
DEFS += HOST_EMULATION
DEFS += _HOST_BUILD

# get the common make flags from test/src/<plat>/../concerto_common.mak
include $($(_MODULE)_SDIR)/../concerto_common.mak

# override CC so that build uses g++-5
# override CC := g++-5

# OPENCV
ifeq ($(BUILD_WITH_OPENCV), 1)
DEFS += BUILD_WITH_OPENCV

# search path for opencv includes
IDIRS += $(TIDL_OPENCV_PATH)/modules/core/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/highgui/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/imgcodecs/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/videoio/include
IDIRS += $(TIDL_OPENCV_PATH)/modules/imgproc/include
IDIRS += $(TIDL_OPENCV_PATH)/cmake

# search path for opencv library
LDIRS += $(TIDL_OPENCV_PATH)/cmake/lib
LDIRS += $(TIDL_OPENCV_PATH)/cmake/3rdparty/lib

# opencv libraries
STATIC_LIBS += opencv_imgproc
STATIC_LIBS += opencv_imgcodecs
STATIC_LIBS += opencv_core
STATIC_LIBS += libtiff
STATIC_LIBS += libwebp
STATIC_LIBS += libpng
STATIC_LIBS += libjpeg-turbo
STATIC_LIBS += IlmImf
STATIC_LIBS += zlib
STATIC_LIBS += libjasper
# opencv needs libdl.so
SHARED_LIBS += dl

# This compiler keeps screaming about warnings
CFLAGS += -std=c++11

# override CC so that build uses g++-5 (for Ubuntu 18.04) or g++-11 (for Ubuntu 22.04)
override CC := g++-$(TIDL_GCC_VERSION)

endif

ifneq ($(BUILD_WITH_OPENACC),yes)
CFLAGS += -Wno-unused-variable \
	      -Wno-maybe-uninitialized \
 	      -Wno-parentheses \
          -Wno-unused-result \
          -Wno-implicit-function-declaration \
          -Wno-int-to-pointer-cast \
          -Wno-int-conversion
endif

include $(FINALE)

endif
